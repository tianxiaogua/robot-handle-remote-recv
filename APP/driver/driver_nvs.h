/*
 * led.h
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */

#ifndef DRIVER_VNS_H
#define DRIVER_VNS_H

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

void driver_nvs_set_u32(nvs_handle nvs_handler, char* namespace_name, char* key, uint32_t value);
void driver_nvs_get_u32(nvs_handle nvs_handler, char* namespace_name, char* key, uint32_t* value);

#endif /* MAIN_LED_H_ */
