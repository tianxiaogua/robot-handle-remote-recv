/*
 * tcp.c
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */
// 参考 https://www.jianshu.com/p/b312120fb686

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "softAP.h"
#include "driver_tool.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

TaskHandle_t Handle_socket_task = NULL; //创建句柄
SOFTAP_CFG g_softAP_config;


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        GUA_LOGI("station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        GUA_LOGE("station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
        if (g_softAP_config.socket_client_fd > 0) {
            GUA_LOGE("wifi disconnect, close socket:%d", g_softAP_config.socket_client_fd);
            shutdown(g_softAP_config.socket_client_fd, 0);
            close(g_softAP_config.socket_client_fd);
        }
    }
}

static int32 wifi_init_softap(void)
{
    // 1. 创建AP工作模式
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    // 2.设置WiFi工作信息
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    GUA_LOGI("wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
    
    return REV_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int32 tcp_send_data(uint8 *data, uint32 data_len)
{
    int to_write = data_len;

    if (g_softAP_config.socket_client_fd == -1) {
        GUA_LOGE("socket is shutdown!");
        return REV_ERR;
    }

    while (to_write > 0) {
        int written = send(g_softAP_config.socket_client_fd, data + (data_len - to_write), to_write, 0);
        if (written < 0) {
            GUA_LOGE("Error occurred during sending: errno %d", errno);
            return REV_ERR;
        }
        to_write -= written;
    }
    return REV_OK;
}

static int32 tcp_recv_data(const int sock)
{
    int32 ret = 0;
    while (1) {
    	ret = recv(sock, g_softAP_config.recv_buffer, sizeof(g_softAP_config.recv_buffer) -1, 0);
        if (ret < 0) {
            GUA_LOGE("Error occurred during receiving: errno %d", errno);
            GUA_LOGE("shut down socket fd: %d", sock);
            return REV_ERR;
        } else if (ret == 0) {
            GUA_LOGE("Connection closed");
            return REV_ERR;
        } else {
            g_softAP_config.recv_len = ret;
            g_softAP_config.recv_buffer[g_softAP_config.recv_len] = 0; // 将接收到的任何内容终止为空，并将其视为字符串!
            g_softAP_config.recv_data_callback(g_softAP_config.recv_buffer, g_softAP_config.recv_len);

            GUA_LOGI("Received %d bytes: %s", g_softAP_config.recv_len, g_softAP_config.recv_buffer);            
        }
    }
    return REV_ERR;
}

static void tcp_init_server(void * pvParameters)
{
    char addr_str[SOCKET_RECV_BUF_LEN];
    int addr_family = AF_INET;
    
    int listen_sock = 0;
    int ret = 0;
    struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    struct sockaddr_in6 dest_addr;
	struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    g_softAP_config.socket_client_fd = 0;

    // 创建socket server

    // 1. 创建socket标识符
	GUA_LOGI("start socket");
    listen_sock = socket(addr_family, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        GUA_LOGE("Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }
    GUA_LOGI("Socket created");

    // 2. 绑定端口
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr_ip4->sin_family = AF_INET;
	dest_addr_ip4->sin_port = htons(PORT);
    ret = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret != 0) {
        GUA_LOGE("Socket unable to bind: errno %d", errno);
        GUA_LOGE("IPPROTO: %d", addr_family);
        close(listen_sock);
        vTaskDelete(NULL);
    }
    GUA_LOGI("Socket bound, port %d", PORT);

    // 3. 开启监听
    ret = listen(listen_sock, 1);
    if (ret != 0) {
        GUA_LOGE("Error occurred during listen: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
    }

    // 4. 启动接收连接 如果断开连接就重新等待接受连接
    while (1) {
        GUA_LOGI("Socket listening...");

        g_softAP_config.socket_client_fd = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len); // 阻塞等待获取连接
        if (g_softAP_config.socket_client_fd < 0) {
            GUA_LOGE("Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        GUA_LOGI("Socket accepted ip address: %s", addr_str);

        ret = tcp_recv_data(g_softAP_config.socket_client_fd);
        if (ret == REV_ERR) {
            shutdown(g_softAP_config.socket_client_fd, 0);
            close(g_softAP_config.socket_client_fd);
            g_softAP_config.socket_client_fd = -1;
            GUA_LOGE("close socket");
        }
    }
}


/**
 * @brief 初始化softAP模块
 * 
 * @return int32 
 */
int32 cmp_softAP_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    GUA_LOGI("ESP_WIFI_MODE_AP");
    wifi_init_softap();
    return REV_OK;
}

/**
 * @brief 创建基于AP模式下的socket的server端
 * 
 * @return int32 
 */
int32 cmp_softAP_tcp_init_server(void)
{
    int32 ret = REV_OK;
    // 创建任务
	xTaskCreatePinnedToCore(tcp_init_server,            //任务函数
							"init_tcp_server",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							1,                   //任务优先级
							&Handle_socket_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
    return ret;
}

/**
 * @brief 创建基于AP模式下的socket的server端
 * 
 * @return int32 
 */
void cmp_softAP_tcp_register_recv(cmp_softAP_recv_data_callback fun_cb)
{
    g_softAP_config.recv_data_callback = fun_cb;
}

/**
 * @brief 通过网络发送数据
 * 
 * @return int32 
 */
int32 cmp_softAP_tcp_send_data(uint8 *buf, uint32 buf_len)
{
    int32 ret = 0;
    ret = tcp_send_data(buf, buf_len);
    return ret;
}
