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

#include "protocol.h"

#define TCP_DATA_LEN 512
typedef struct
{
	uint8 tcp_data_buf[TCP_DATA_LEN];
} APP_CONFIG;

APP_CONFIG g_app_config;

TaskHandle_t Handle_usb_task = NULL;
static int32 app_start_usb_task(void)
{
	// 创建USB控制任务
	xTaskCreatePinnedToCore(app_tinyusb,            //任务函数
							"usb_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							2,                   //任务优先级
							&Handle_usb_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
	return REV_OK;
}

#define SOFT_FRO_GAMEPAD_RECV
#ifndef SOFT_FRO_GAMEPAD_RECV
/**
 * 以下为遥控器部分的代码
 * */


/**
 * 用于在接收到数据时处理
 * */

KEY_DETECTION key_data;

static void softAP_recv_data_callback(uint8 *recv_buffer, uint32 recv_len)
{
	char json_buf[256];
	GUA_LOGI("callback: data len:%d", recv_len);
	if (recv_len > 256) {
		GUA_LOGE("recv data length is too long!");
		return;
	}
	strcpy(json_buf, (char *)recv_buffer);
	protocol_decode(json_buf, &key_data);
	printf("UP:%d DOWN:%d LEFT:%d RIGHT:%d A:%d B:%d X:%d Y:%d LB:%d RB:%d START:%d SELECT:%d R_KEY:%d L_KEY:%d  %d %d %d %d\r\n",
		key_data.KEY_UP_,
		key_data.KEY_DOWN_,
		key_data.KEY_LEFT,
		key_data.KEY_RIGHT,
		key_data.KEY_A,
		key_data.KEY_B,
		key_data.KEY_X,
		key_data.KEY_Y,
		key_data.KEY_LB,
		key_data.KEY_RB,
		key_data.KEY_START,
		key_data.KEY_SELECT,
		key_data.KEY_R_KEY,
		key_data.KEY_L_KEY,
		key_data.LX,
		key_data.LY,
		key_data.RX,
		key_data.RY
	);

	memset(json_buf, 0, sizeof(json_buf));
	protocol_encode(json_buf , &key_data);

	cmp_softAP_tcp_send_data((uint8 *)json_buf, strlen(json_buf));
}
#endif
#ifdef SOFT_FRO_GAMEPAD_RECV
int32 app_wifi_event_handle_cb(int32 evint_handle)
{
	int32 ret = 0;
	
	switch (evint_handle) {
	case WIFI_CONNECTED:
		ret = cmp_wifi_tcp_client_init();
		break;
	case WIFI_TCP_RECV_DATA:
		ret = cmp_wifi_recv_data(g_app_config.tcp_data_buf, TCP_DATA_LEN);
		if (ret != REV_ERR) { // 数据正确的时候处理数据
			cmp_wifi_send_data(g_app_config.tcp_data_buf, ret);
		}
		break;
	case WIFI_TCP_DISCONNECTED:
		cmp_wifi_tcp_client_deinit();
		break;
	default:
		GUA_LOGE("event callback is error!");
		break;
	}
	return ret;
}
#endif


void start_app(void)
{
	int32 ret = 0;

	printf("\n\n-----------------ESP32 GAMEPAD----------------------\nV1.0.0 -HRAD V1.3\n\n");
	driver_init_gpio();

	ret = app_start_usb_task();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}

#ifdef SOFT_FRO_GAMEPAD_RECV // 软件版本设置为接收器
	cmp_wifi_init_sta();
	cmp_wifi_event_handle_register(app_wifi_event_handle_cb);
#else // 遥控器部分代码
	ret = cmp_softAP_init();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}
	cmp_softAP_tcp_init_server();
	cmp_softAP_tcp_register_recv(softAP_recv_data_callback);

#endif
	while (1) {
		delay_ms(100000);
	}
}


