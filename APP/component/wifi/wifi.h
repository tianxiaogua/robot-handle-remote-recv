#ifndef APP_WIFI_WIFI_H_
#define APP_WIFI_WIFI_H_


#include "main_app.h"
#include "driver_tool.h"

enum wifi_config_e
{
    WIFI_CONNECTED = 0,
    WIFI_TCP_RECV_DATA,
	WIFI_TCP_DISCONNECTED
};

typedef int32 (*cmp_wifi_event_handle_callback)(int32 evint_handle); // 事件处理

typedef struct 
{
	int32 tcp_socket_client_fd;
    cmp_wifi_event_handle_callback event_handle_callback;
    int32 udp_client_fd;
    int32 type;
} WIFI_CFG;


/**
 * @brief 初始化wifi
 * 
 * @return int32 
 */
int32 cmp_wifi_init_sta(void);

/**
 * @brief 创建socket的server端
 * 
 * @return int32 
 */
int32 cmp_wifi_tcp_server_init(void);

/**
 * @brief socket的client端初始化    
 * 
 * @return int32 
 */
int32 cmp_wifi_socket_client_init(void);

/**
 * @brief socket的client端注销
 * 
 * @return int32 
 */
int32 cmp_wifi_tcp_client_deinit(void);

/**
 * @brief 注册接收回调函数
 * 
 * @return int32 
 */
int32 cmp_wifi_event_handle_register(cmp_wifi_event_handle_callback fun_cb);

/**
 * @brief 通过网络发送数据
 * 
 * @return int32 
 */
int32 cmp_wifi_send_data(uint8 *buf, uint32 buf_len);

/**
 * @brief 接受网络数据
 * 
 * @param buf 接受数据
 * @param buf_len 接受数据buf长度
 * @return int32 接收到的数据长度
 */
int32 cmp_wifi_recv_data(uint8 *buf, uint32 buf_len);

#endif /* APP_WIFI_WIFI_H_ */
