/*
 * nvs.h
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 */

#ifndef APP_NVS_NVS_H_
#define APP_NVS_NVS_H_


#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include <esp_wifi.h>

#define NAMESPACE_WIFI "WiFi_cfg"  // 储存空间命名 这里是用于存放WiFi用户密码的数据
#define NAMESPACE_KEY_WIFI_SSID "wifi_ssid"
#define NAMESPACE_KEY_WIFI_PAWD "wifi_passwd"


esp_err_t vns_read_wificfg(wifi_config_t *wifi_config);
void vns_save_wificfg(const char *ssid, const char *password);

#endif /* APP_NVS_NVS_H_ */
