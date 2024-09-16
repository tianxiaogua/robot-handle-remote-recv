/*
 * main_app.c
 *
 *  Created on: 2024年7月14日
 *      Author: tianxiaohua
 */

#include "main_app.h"
#include "driver_nvs.h"
#include "driver_pwm.h"
#include "app_tinyusb.h"
#include "driver_timer.h"
#include "driver_pwm_beep.h"
#include "driver_tool.h"

#include "wifi.h"
#include "nvs.h"
#include "softAP.h"



static USB_OUT_CFG usb_out_config;


TaskHandle_t Handle_APP_task = NULL; //创建句柄
TaskHandle_t Handle_usb_task = NULL;
TaskHandle_t Handle_wifi_task = NULL;



// 获取接收器模式 USB输出为串口模式、USB输出为手柄模式
static int32 app_get_usb_model(void)
{
	// 获取按钮输入
	return USB_SERIVAL_MODE;
}


// 任务：实现主要的应用部分逻辑
static void app_recv_usb_handle(void * pvParameters)
{
	int32 ret = 0;

	usb_out_config.set_usb_mode = app_get_usb_model();

	ret = app_set_usb_mode(&usb_out_config);
	if (ret != REV_OK) {

	}

	while(1)
	{
		delay_ms(1000);
	}
}

void usb_task(void)
{
	// 创建USB控制任务
	xTaskCreatePinnedToCore(app_tinyusb,            //任务函数
							"usb_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							3,                   //任务优先级
							&Handle_usb_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
}

 // 用于在接收到数据时处理
static void softAP_recv_data_callback(uint8 *recv_buffer, uint32 recv_len)
{
	GUA_LOGI("callback: data len:%d", recv_len);
	cmp_softAP_send_data(recv_buffer, recv_len);
}




esp_err_t init_app(void)
{
//	int32 ret = 0;
	printf("\n\n-----------------ESP32 GAMEPAD----------------------\nV1.0.0 -HRAD V1.3\n\n");
	driver_init_gpio();

	return ESP_OK;
}


void start_app(void)
{
	int32 ret = 0;
	// 创建LCD控制任务
	xTaskCreatePinnedToCore(app_recv_usb_handle,            //任务函数
							"app_recv_usb_handle",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							1,                   //任务优先级
							&Handle_APP_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上

	usb_task();
	ret = cmp_softAP_init();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}
	cmp_softAP_bulid_tcp_server();
	cmp_softAP_register_recv(softAP_recv_data_callback);
	while (1)
	{
		delay_ms(100000);
	}
	
}


