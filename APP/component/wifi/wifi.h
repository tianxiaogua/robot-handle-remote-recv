/*
 * wifi.h
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 */

#ifndef APP_WIFI_WIFI_H_
#define APP_WIFI_WIFI_H_

#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif_ip_addr.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


void wifi_connect(wifi_config_t *wifi_config);
void set_wifi_connect_state(int state);
int get_wifi_connect_state(void);

void tcp_client_handle(void);
void tcp_init_server(void);

#endif /* APP_WIFI_WIFI_H_ */
