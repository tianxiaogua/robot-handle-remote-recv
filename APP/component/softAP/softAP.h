
#ifndef SOFTAP_H_
#define SOFTAP_H_

#include "main_app.h"
#include "driver_tool.h"

#define EXAMPLE_ESP_WIFI_SSID      "ESP32_TEST"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4
#define PORT 3333

#define SOCKET_RECV_BUF_LEN 512

typedef void (*cmp_softAP_recv_data_callback)(uint8 *recv_buffer, uint32 recv_len); // 用于在接收到数据时处理
typedef struct 
{
    /* data */
    cmp_softAP_recv_data_callback recv_data_callback;
    uint32 recv_len;
    uint8 recv_buffer[SOCKET_RECV_BUF_LEN];
    int32 socket_client_fd;
}SOFTAP_CFG;


/**
 * @brief 初始化softAP模块
 * 
 * @return int32 
 */
int32 cmp_softAP_init(void);

/**
 * @brief 创建基于AP模式下的socket的server端
 * 
 * @return int32 
 */
int32 cmp_softAP_bulid_tcp_server(void);

/**
 * @brief 创建基于AP模式下的socket的client端
 * 
 * @return int32 
 */
// int32 cmp_softAP_bulid_tcp_client(SOFTAP_CFG *cfg);

/**
 * @brief 注册接收回调函数
 * 
 * @return int32 
 */
void cmp_softAP_register_recv(cmp_softAP_recv_data_callback fun_cb);

/**
 * @brief 通过网络发送数据
 * 
 * @return int32 
 */
int32 cmp_softAP_send_data(uint8 *buf, uint32 buf_len);

#endif
