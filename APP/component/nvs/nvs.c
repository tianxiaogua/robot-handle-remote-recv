/*
 * nvs.c
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 */
#include "nvs.h"

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





/**************************************************
 * @函数： vns_save_wificfg
 * @作者： 田小呱
 * @时间： 23/8
 * 概述： 储存WiFi的ssid和password在flash中
 * 输入： 要储存的WiFi的ssid和password
 * 输出： 无
 ***************************************************/
void vns_save_wificfg(const char *ssid, const char *password)
{
	ESP_ERROR_CHECK( nvs_flash_init() );

	ESP_LOGI("nvs flash save config", "wifi:%s ssid:%s", ssid, password);
	/* 将得到的WiFi名称和密码存入NVS*/
	nvs_handle wificfg_nvs_handler;
	ESP_ERROR_CHECK( nvs_open(NAMESPACE_WIFI, NVS_READWRITE, &wificfg_nvs_handler) ); // 打开一个命名的存储空间
	ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler,NAMESPACE_KEY_WIFI_SSID,ssid) ); // 设置储存的键值为字符串
	ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler,"wifi_passwd",password) );
	ESP_ERROR_CHECK( nvs_commit(wificfg_nvs_handler) ); /* 提交 */
	nvs_close(wificfg_nvs_handler);                     /* 关闭 */
	ESP_LOGI("nvs flash save config", "smartconfig save wifi_cfg to NVS .\n");
}

/**************************************************
 * @函数： vns_read_wificfg
 * @作者： 田小呱
 * @时间： 23/8
 * 概述： 读取储存在flash中的wifi的ssid和password
 * 输入： 储存WiFi数据的结构体指针
 * 输出： 状态
 ***************************************************/
esp_err_t vns_read_wificfg(wifi_config_t *wifi_config)
{
	esp_err_t rev ;

	ESP_ERROR_CHECK( nvs_flash_init() );

	nvs_handle wificfg_nvs_handler_rev; /* 定义一个NVS操作句柄 */
	char wifi_ssid[32] = { 0 };     /* 定义一个数组用来存储ssid*/
	char wifi_passwd[64] = { 0 };   /* 定义一个数组用来存储passwd */
	size_t len;
	/* 打开一个NVS命名空间 */
	rev = nvs_open(NAMESPACE_WIFI, NVS_READONLY, &wificfg_nvs_handler_rev);
	if(rev != ESP_OK) {
		ESP_LOGW("NVS READ", "not message");
		return ESP_FAIL;
	}
	len = sizeof(wifi_ssid);    /* 从NVS中获取ssid */
	ESP_ERROR_CHECK( nvs_get_str(wificfg_nvs_handler_rev,"wifi_ssid", (char *)&wifi_config->sta.ssid, &len) );
	len = sizeof(wifi_passwd);      /* 从NVS中获取ssid */
	ESP_ERROR_CHECK( nvs_get_str(wificfg_nvs_handler_rev,"wifi_passwd", (char *)&wifi_config->sta.password, &len) );

	ESP_ERROR_CHECK( nvs_commit(wificfg_nvs_handler_rev) ); /* 提交 */
	nvs_close(wificfg_nvs_handler_rev);                     /* 关闭 */

	return ESP_OK;
}




