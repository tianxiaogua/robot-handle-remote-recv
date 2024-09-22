/*
 * wifi.c
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 *  参考： https://github.com/espressif/esp-idf/blob/3c99557eeea4e0945e77aabac672fbef52294d54/examples/wifi/scan/main/scan.c#L180
 */

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

#define DEFAULT_SCAN_LIST_SIZE 20

#define DEFAULT_WIFI_SSID "ESP32_TEST"
#define DEFAULT_WIFI_PASS "12345678"

#define SOCKET_SERVER_IP "192.168.4.1"
#define SOCKET_SERVER_PORT  3333

#define WIFI_STATE_WAIT 0
#define WIFI_STATE_FALE 1
#define WIFI_STATE_SUCS 2

enum SOCKET_TPYE
{
	SOCKET_UPD = 1,
	SOCKET_TCP
};

WIFI_CFG g_wifi_config;
TaskHandle_t Handle_wifi_task = NULL;
static struct sockaddr_in udp_client_addr;                  //client地址

static int wifi_state = WIFI_STATE_WAIT;

static void set_wifi_connect_state(int state)
{
	wifi_state = state;
}

static int get_wifi_connect_state(void)
{
	return wifi_state;
}

/**
 * @brief 用于初始化nvs
 */
static void init_nvs() 
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    	GUA_LOGI("begin to connect :event_base:%d event_id:%d", (int)event_base, (int)event_id);
		set_wifi_connect_state(WIFI_STATE_WAIT);
    }

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        GUA_LOGI("Got IP: " IPSTR,  IP2STR(&event->ip_info.ip));
        GUA_LOGI("successful event_base:%d event_id:%d", (int)event_base, (int)event_id);
        set_wifi_connect_state(WIFI_STATE_SUCS);
    }

    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    	GUA_LOGE("Connect error event_base:%d event_id:%d", (int)event_base, (int)event_id);
		set_wifi_connect_state(WIFI_STATE_FALE);
	}
}


static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	esp_wifi_set_ps (WIFI_PS_NONE); // 关闭WiFi低功耗 否则将会造成至少50ms以上的延时
	
	esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
	esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);
	
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}


static int32 wifi_scan(char *target_ssid)
{
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));

	esp_wifi_scan_start(NULL, true);

	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

	GUA_LOGI("wifi_scan: total APs scanned = %u, actual AP number ap_info holds = %u find ssid:%s", ap_count, number, target_ssid);
	for (int i = 0; i < number; i++) {
		GUA_LOGI("SSID:%s   RSSI:%d", ap_info[i].ssid, ap_info[i].rssi);
		if (strcmp((char *)ap_info[i].ssid, target_ssid) == 0) {
			GUA_LOGI("get target ssid successful!");
			return REV_OK;
		}
	}
	return REV_ERR;
}


static int32 wifi_connect(void)
{
	wifi_config_t wifi_config;
	int32 ret = 0;

	strcpy((char *)wifi_config.sta.ssid, DEFAULT_WIFI_SSID);
	strcpy((char *)wifi_config.sta.password, DEFAULT_WIFI_PASS);

	GUA_LOGI("ssid:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
	esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

	ret = esp_wifi_connect();
	if (ret == ESP_OK) {
		GUA_LOGI("wifi connected ok! wifi ret =%d", ret);
		return REV_OK;
	} else {
		GUA_LOGE("wifi connsct ret =%d", ret);
		return REV_ERR;
	}
}

static int32 sta_wifi_tcp_client_event(int32 tcp_status)
{
	int32 ret = 0;

	if (tcp_status == 0) {
		ret = g_wifi_config.event_handle_callback(WIFI_CONNECTED);
		if (ret == REV_OK) {
			tcp_status = 1;
		}
	}

	ret = g_wifi_config.event_handle_callback(WIFI_TCP_RECV_DATA);
	if (ret == REV_ERR) { // TCP断开了重新连接TCP
		g_wifi_config.event_handle_callback(WIFI_TCP_DISCONNECTED);
		tcp_status = 0;
	}

	return tcp_status;
}


static int32 sta_wifi_udp_client_event(int32 tcp_status)
{
	int32 ret = 0;

	if (tcp_status == 0) {
		ret = g_wifi_config.event_handle_callback(WIFI_CONNECTED);
		if (ret == REV_OK) {
			tcp_status = 1;
		}
	}

	ret = g_wifi_config.event_handle_callback(WIFI_TCP_RECV_DATA);
	if (ret == REV_ERR) { // TCP断开了重新连接TCP
		g_wifi_config.event_handle_callback(WIFI_TCP_DISCONNECTED);
		tcp_status = 0;
	}

	return tcp_status;
}


static void sta_connect_wifi(void * pvParameters)
{
	int32 ret = 0;
	int32 tcp_status = 0;
	g_wifi_config.type = (int) pvParameters;

	init_nvs();
	wifi_init();
	while (1) {
		while (1) {
			ret = wifi_scan(DEFAULT_WIFI_SSID);
			if (ret == REV_OK) {
				ret = wifi_connect();
				if (ret == REV_OK) {
					break;
				} 
				delay_ms(2000); // 尝试两次
				ret = wifi_connect();
				if (ret == REV_OK) {
					break;
				}
			}
			delay_ms(100);
		}
		GUA_LOGI("wifi conneced wait to connect socket...");
		while (1) {
			ret = get_wifi_connect_state();
			if (ret == WIFI_STATE_SUCS) {
				if (g_wifi_config.type == SOCKET_UPD) {
					tcp_status = sta_wifi_tcp_client_event(tcp_status); // TCP 事件处理
				} else {
					tcp_status = sta_wifi_udp_client_event(tcp_status); // UDP 事件处理
				}
			} else if (ret == WIFI_STATE_WAIT) {
				GUA_LOGW("wifi just wait connect...");
				delay_ms(1000);
			} else {
				GUA_LOGE("wifi disconnected!");
				g_wifi_config.event_handle_callback(WIFI_TCP_DISCONNECTED);
				break;
			}
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int32 tcp_client_recv_data(uint8 *buf, uint32 buf_len)
{
	int32 len = 0;

	len = recv(g_wifi_config.tcp_socket_client_fd, buf, buf_len, 0);  // 读取接收数据
	if(len < 0) { // 没有数据

	} else if (len == 0) {
		GUA_LOGW("Connection closed");
		return REV_ERR;
	} else {
		GUA_LOGI("Received %d bytes: %s", len, buf);
		return len;
	}
	return REV_ERR;
}

static int32 tcp_client_send_data(uint8 *buf, uint32 buf_len)
{
	int32 ret = 0;

	ret = send(g_wifi_config.tcp_socket_client_fd, buf, buf_len, 0);
	if(ret < 0) {
		GUA_LOGE("send err. \n");
		ret = REV_ERR;
	}
	return ret;
}

static int32 tcp_client_handle(void)
{
    int ret = -1;

    GUA_LOGI("socket tcp connect to server ...");
    /* 打开一个socket套接字 */
    g_wifi_config.tcp_socket_client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_wifi_config.tcp_socket_client_fd < 0) {
		GUA_LOGE("socket open failure ! sockfd:%d", g_wifi_config.tcp_socket_client_fd);
		return REV_ERR;
	}
	
	struct sockaddr_in seraddr = {0};
	seraddr.sin_family = AF_INET;		                    /* 设置地址族为IPv4 */
	seraddr.sin_port = htons(SOCKET_SERVER_PORT);	        /* 设置地址的端口号信息 */
	seraddr.sin_addr.s_addr = inet_addr(SOCKET_SERVER_IP);	/*　设置IP地址 */
	ret = connect(g_wifi_config.tcp_socket_client_fd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
	if(ret < 0) {
		GUA_LOGE("connect to server failure ! ret = %d",ret);
		return REV_ERR;
	}
	GUA_LOGI("tcp client connect success!\n");

	return REV_OK;
}


static void tcp_init_server(void)
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
	dest_addr.sin_port = htons(SOCKET_SERVER_PORT);

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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int32 udp_client_recv_data(uint8_t *data_buf, uint32 data_len)
{
    int len = 0;            //长度
    //创建udp客户端
    socklen_t udpsocklen = sizeof(udp_client_addr);      //地址长度
	//读取接收数据
	len = recvfrom(g_wifi_config.udp_client_fd, data_buf, data_len, 0, (struct sockaddr *) &udp_client_addr, &udpsocklen);
	if (len > 0) {
		return len;
	} else {
		//断开连接
		GUA_LOGE("udp client connect error!");
		return REV_ERR;
	}
	GUA_LOGE("udp client connect error!");

	return REV_ERR;
}


static int32 udp_client_send_data(uint8_t *data_buf, uint32 data_len)
{
	int len = 0;            //长度
	unsigned int udpsocklen = sizeof(udp_client_addr);      //地址长度
	//测试udp server,返回发送成功的长度
	len = sendto(g_wifi_config.udp_client_fd, data_buf, data_len, 0, (struct sockaddr *) &udp_client_addr, udpsocklen);
	if (len > 0) {
		return ESP_OK;
	} else {
		GUA_LOGE("udp client send error!");
		return ESP_FAIL;
	}
	GUA_LOGE("udp client send error!");

	return ESP_OK;
}


static int32 init_udp_client_handle(void)
{
	g_wifi_config.udp_client_fd = -1;

	//新建socket
	g_wifi_config.udp_client_fd = socket(AF_INET, SOCK_DGRAM, 0);                         /*参数和TCP不同*/
	if (g_wifi_config.udp_client_fd < 0) {
		//新建失败后，关闭新建的socket，等待下次新建
		close(g_wifi_config.udp_client_fd);
		GUA_LOGE("udp client init error!");
		return ESP_FAIL;
	}

	//配置连接服务器信息
	udp_client_addr.sin_family = AF_INET;
	udp_client_addr.sin_port = htons(SOCKET_SERVER_PORT);
	udp_client_addr.sin_addr.s_addr = inet_addr(SOCKET_SERVER_IP);
	GUA_LOGI("udp client init ok");
	
	uint8 buf[] = "connected\n";
	udp_client_send_data(buf, sizeof(buf));

	return ESP_OK;
}



/////////////////////////////////////外部接口////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 初始化wifi
 * 
 * @return int32 
 */
int32 cmp_wifi_init_sta(void)
{
	GUA_LOGI("start wifi task");
	// 创建wifi task控制任务
	xTaskCreatePinnedToCore(sta_connect_wifi,            //任务函数
							"wifi_task",          //任务名称
							4096+2048,                //堆栈大小
							(void *)SOCKET_UPD,                //传递参数
							1,                   //任务优先级
							&Handle_wifi_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
	return REV_OK;
}

/**
 * @brief 创建socket的server端
 *
 * @return int32
 */
int32 cmp_wifi_tcp_server_init(void)
{
	tcp_init_server();
	return REV_OK;
}

int32 cmp_wifi_socket_client_init(void)
{
	if (g_wifi_config.type == SOCKET_UPD) {
		return init_udp_client_handle();
	}
	if (g_wifi_config.type == SOCKET_TCP) {
		return tcp_client_handle();
	}
	return REV_ERR;
}

/**
 * @brief socket的client端注销
 * 
 * @return int32 
 */
int32 cmp_wifi_tcp_client_deinit(void)
{
	GUA_LOGW("disconnect tcp");
	return close(g_wifi_config.tcp_socket_client_fd);
}


/**
 * @brief 注册接收回调函数
 * 
 * @return int32 
 */
int32 cmp_wifi_event_handle_register(cmp_wifi_event_handle_callback fun_cb)
{
	if (fun_cb == NULL) {
		GUA_LOGE("wifi event register function is NULL!");
		return REV_ERR;
	}
	g_wifi_config.event_handle_callback = fun_cb;
	return REV_OK;
}

/**
 * @brief 通过网络发送数据
 * 
 * @return int32 
 */
int32 cmp_wifi_send_data(uint8 *buf, uint32 buf_len)
{	
	int32 ret = 0;
	
	if (g_wifi_config.type == SOCKET_UPD) {
		ret = udp_client_send_data(buf, buf_len);

	} else {
		ret = tcp_client_send_data(buf, buf_len);
	}
	return ret;
}

/**
 * @brief 接受网络数据
 * 
 * @param buf 接受数据
 * @param buf_len 接受数据buf长度
 * @return int32 接收到的数据长度
 */
int32 cmp_wifi_recv_data(uint8 *buf, uint32 buf_len)
{
	if (g_wifi_config.type == SOCKET_UPD) {
		return udp_client_recv_data(buf, buf_len);
	} else {
		return tcp_client_recv_data(buf, buf_len);
	}

	return REV_ERR;
}
