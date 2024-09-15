/*
 * main_app.h
 *
 *  Created on: 2024年7月14日
 *      Author: tianxiaohua
 */

#ifndef APP_MAIN_APP_H_
#define APP_MAIN_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_log.h"

#include "driver_gpio.h"
#include "driver_tool.h"
#include "driver_spi.h"
#include "driver_adc.h"
#include "driver_iic.h"


#define USB_SERIVAL_MODE 0x01 // 接收器USB串口模式
#define USB_HANDLE_MODE  0x02 // 接收器USB手柄模式

typedef struct
{
	uint8 usb_out_mode; // usb输出模式
	uint8 set_usb_mode; // 设置控制模式开关
} USB_OUT_CFG;

esp_err_t init_app(void);
void start_app(void);


#endif /* APP_MAIN_APP_H_ */
