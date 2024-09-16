/*
 * wifi.c
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 */
// 参考 https://www.jianshu.com/p/b312120fb686
#include "driver_tool.h"
#include "nvs.h"

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

#include "wifi.h"
#include "driver_tool.h"

#define DEFAULT_WIFI_NAME "tianxiaohuawifi"
#define DEFAULT_WIFI_PASS "88888888"
#define WIFI_STATE_WAIT 0
#define WIFI_STATE_FALE 1
#define WIFI_STATE_SUCS 2

/**
 * @brief 用于初始化nvs
 */
void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/**
 * @brief WiFi 的事件循环Handler
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
// https://blog.csdn.net/m0_50064262/article/details/119999907
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    	int temp_value = esp_wifi_connect();
    	GUA_LOGI("begin to connect ：event_base:%d event_id:%d :%d", (int)event_base, (int)event_id, temp_value);
    }

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        GUA_LOGI("Got IP: " IPSTR,  IP2STR(&event->ip_info.ip));
        GUA_LOGI("successful ！！！！！！event_base:%d event_id:%d", (int)event_base, (int)event_id);
        set_wifi_connect_state(WIFI_STATE_SUCS);
    }

    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    	GUA_LOGE("Connect error ！！ event_base:%d event_id:%d", (int)event_base, (int)event_id);
		set_wifi_connect_state(WIFI_STATE_FALE);
	}
    GUA_LOGI("event_base:%d event_id:%d", (int)event_base, (int)event_id);
}

void wifi_connect(wifi_config_t *wifi_config)
{
	set_wifi_connect_state(WIFI_STATE_WAIT);

	ESP_LOGI("wifi_task", "wait to connect wifi\r\n");

    init_nvs();

    esp_netif_init();

    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    GUA_LOGI("ssid:%s password:%s\r\n", wifi_config->sta.ssid, wifi_config->sta.password);
    //  而直接将wifi_sta_config_t(或指针)转为wifi_config_t(或指针)是GCC的拓展语法，如下
    esp_wifi_set_config(WIFI_IF_STA, wifi_config);

    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);

    esp_wifi_start();
}


int wifi_state = WIFI_STATE_WAIT;
void set_wifi_connect_state(int state)
{
	wifi_state = state;
}

int get_wifi_connect_state(void)
{
	return wifi_state;
}



void wifi_init(void)
{
	int temp_recv = 0;
	wifi_config_t cfg_sta;
	wifi_config_t _cfg_sta;

	temp_recv = vns_read_wificfg(&cfg_sta); // 初始尝试读取flash上面的WiFi配置
	if(temp_recv != ESP_OK){  // 如果没有读取到flash上面的WiFi配置
		GUA_LOGI("no wifi config");
		vns_save_wificfg(DEFAULT_WIFI_NAME, DEFAULT_WIFI_PASS);
	}
	else{ // 如果从flash中读取到了WiFi的配置信息
		memcpy(&_cfg_sta.sta.ssid, &cfg_sta.sta.ssid, sizeof(cfg_sta.sta.ssid));
		memcpy(&_cfg_sta.sta.password, &cfg_sta.sta.password, sizeof(cfg_sta.sta.password));
		wifi_connect(&_cfg_sta);
		while(1){
			temp_recv = get_wifi_connect_state();
			if (temp_recv == WIFI_STATE_SUCS) {
				GUA_LOGI("wait to connect web server!!!!!");
				break;
			} else if (temp_recv == WIFI_STATE_WAIT) {
				GUA_LOGW("just wait\r\n");
			} else {
				GUA_LOGE("no no no no no no !");
			}
			delay_ms(1000);
		}
	}
}


#define SOCKET_SERVER_IP "192.168.0.105"
#define SOCKET_SERVER_PORT  1111
#define TCP_PORT             3333                // TCP服务器端口号
#define PORT TCP_PORT

void tcp_client_handle(void)
{
    int sockfd = -1;    /* 定义一个socket描述符，用来接收打开的socket */
    int ret = -1;
    /* 打开一个socket套接字 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
		GUA_LOGE("socket open failure ! sockfd:%d \n", sockfd);
		return ;
	} else {
        struct sockaddr_in seraddr = {0};
        seraddr.sin_family = AF_INET;		                    /* 设置地址族为IPv4 */
	    seraddr.sin_port = htons(SOCKET_SERVER_PORT);	                    /* 设置地址的端口号信息 */
	    seraddr.sin_addr.s_addr = inet_addr(SOCKET_SERVER_IP);	/*　设置IP地址 */
	    ret = connect(sockfd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
        if(ret < 0) {
        	GUA_LOGE("connect to server failure !!! ret = %d \n",ret);
        } else {
        	GUA_LOGI("connect success.\n");
            int cnt = 10;
            while(cnt--) {
                ret = send(sockfd, "I am ESP32.", strlen("I am ESP32."), 0);
                if(ret < 0) {
                	GUA_LOGE("send err. \n");
                } else {
                	GUA_LOGI("send ok. \n");
                }
                delay_ms(200);
            }
        }
        close(sockfd);
    }
}


void tcp_init_server(void)
{
	int addr_family = AF_INET;
	int ip_protocol = IPPROTO_IP;
	struct sockaddr_in dest_addr;
	int client_fd;

	// 1.新建socket
	int listen_sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
	if(listen_sock < 0) {
		GUA_LOGE("Unable to create socket: errno %d", listen_sock);
	}

	// 2.配置服务器信息
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr.sin_port = htons(TCP_PORT);

	// 3.绑定地址
	int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if(err != 0) {
		GUA_LOGE("Socket unable to bind: errno %d", errno);
		GUA_LOGE("IPPROTO: %d", addr_family);
	    close(listen_sock);
	}
	GUA_LOGI("Socket bound end");

	// 4.开始监听
	err = listen(listen_sock, 1);    // 这里为啥是1，网上大多数是5
	if(err != 0) {
		GUA_LOGE("Error occurred during listen: errno %d", errno);
	    close(listen_sock);
	}

	// 5.等待客户端连接
	while(1) {
	    struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
	    socklen_t addr_len = sizeof(source_addr);
	    client_fd = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len); // 阻塞等待
	    if(client_fd < 0) {
	    	GUA_LOGE("Unable to accept connection: errno %d", errno);
	        close(listen_sock);
	    }
	    break;
	}

	// 接收数据
	int len;
	char rx_buffer[128];
	len = send(client_fd, "I am ESP32.", strlen("I am ESP32."), 0);
	if(len < 0) {
		GUA_LOGE("send err. \n");
	} else {
		GUA_LOGI("send ok. \n");
	}


	GUA_LOGI("start to recv data");


	while(1) {
		memset(rx_buffer, 0, sizeof(rx_buffer));    // 清空缓存
		len = recv(client_fd, rx_buffer, sizeof(rx_buffer), 0);  // 读取接收数据
		if(len < 0) {
			continue;
		}
		else if (len == 0) {
			GUA_LOGW("Connection closed");
		}
		else {
			GUA_LOGI("Received %d bytes: %s", len, rx_buffer);
			len = send(client_fd, "I am ESP32.", strlen("I am ESP32."), 0);
			if(len < 0) {
				GUA_LOGE("send err. \n");
			} else {
				GUA_LOGI("send ok. \n");
			}
		}
		delay_ms(10);
	}
}
