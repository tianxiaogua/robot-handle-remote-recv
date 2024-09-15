/*
 * nvs.c
 *
 *  Created on: 2023年8月22日
 *      Author: tianxiaohua
 */
#include "driver_nvs.h"
#include "driver_tool.h"

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

void driver_nvs_set_u32(nvs_handle nvs_handler, char* namespace_name, char* key, uint32_t value)
{
	esp_err_t ret;

	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		GUA_LOGE("init flash error!");
	}

	ret = nvs_open(namespace_name, NVS_READWRITE, &nvs_handler);
	if (ret != ESP_OK) {
		GUA_LOGE("open flash error!");
	}
	GUA_LOGW("save int data to flash! name:%s key:%s data:%ld", namespace_name, key, value); // 数据保存至flash
	nvs_set_u32(nvs_handler, key, value);
	nvs_commit(nvs_handler);
	nvs_close(nvs_handler);
}


void driver_nvs_get_u32(nvs_handle nvs_handler, char* namespace_name, char* key, uint32_t* value)
{
	esp_err_t ret;

	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		GUA_LOGE("init flash error!");
	}

	ret = nvs_open(namespace_name, NVS_READONLY, &nvs_handler);
	if (ret != ESP_OK) {
		GUA_LOGE("open flash error!");
	}
	nvs_get_u32(nvs_handler, key, value);
	nvs_commit(nvs_handler); /* 提交 */
	nvs_close(nvs_handler);                     /* 关闭 */
}




