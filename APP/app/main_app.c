/*
 * main_app.c
 *
 *  Created on: 2024年7月14日
 *      Author: tianxiaohua
 */
#include <string.h>
// 驱动
#include "driver_nvs.h"
#include "driver_pwm.h"
#include "driver_timer.h"
#include "driver_pwm_beep.h"
#include "driver_tool.h"

// 组件
#include "wifi.h"
#include "nvs.h"
#include "softAP.h"
#include "usb.h"
#include "keyscan.h"

// 应用
#include "protocol.h"
#include "main_app.h"

#define TCP_DATA_LEN 512
typedef struct
{
	uint8 tcp_data_buf[TCP_DATA_LEN];
} APP_CONFIG;

APP_CONFIG g_app_config;

TINYUSB_BUTTON g_button;
TINYUSB_BUTTON g_tinyusb_button_data;
KEY_DETECTION g_gamepad_key;

static char json_buf[256];
KEY_DETECTION key_data;

// {"hd":"H3.1S1.0","v":{"U":1,"D":0,"L":0,"R":0,"A":1,"B":0,"X":0,"Y":0,"LB":0,"RB":0,"ST":0,"SE":0,"RK":0,"LK":0,"LX":127,"LY":127,"RX":127,"RY":127}}
// {"hd":"H3.1S1.0","v":{"U":1,"D":0,"L":0,"R":0,"A":1,"B":0,"X":0,"Y":0,"LB":0,"RB":0,"ST":0,"SE":0,"RK":0,"LK":0,"LX":127,"LY":127,"RX":127,"RY":127}}

#define SOFT_FRO_GAMEPAD_RECV
#ifdef SOFT_FRO_GAMEPAD_RECV
/**
 * 以下为接收器部分的代码
 * */
static int32 app_decode_recv_data(uint8 *recv_buffer, uint32 recv_len)
{
	memset(json_buf, 0, sizeof(json_buf));

	GUA_LOGI("callback: data len:%d", recv_len);
	if (recv_len > 256) {
		GUA_LOGE("recv data length is too long!");
		return REV_ERR;
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
	return REV_OK;
}

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
			app_decode_recv_data(g_app_config.tcp_data_buf, ret); // 把接收到的数据通过json解码为按键数据
			cmp_keyscan_dispose_gamepad_key(&key_data, &g_tinyusb_button_data); // 将按键数据处理成为游戏手柄数据
			cmp_usb_gamepad_send(&g_tinyusb_button_data); // 通过usb把游戏手柄数据发送出去
			cmp_wifi_send_data(g_app_config.tcp_data_buf, ret); // 测试代码
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
#else
/**
 * 以下为遥控器部分的代码
 * */


/**
 * 用于在接收到数据时处理
 * */

static void softAP_recv_data_callback(uint8 *recv_buffer, uint32 recv_len)
{
	app_decode_recv_data(recv_buffer, recv_len);

	memset(json_buf, 0, sizeof(json_buf));
	protocol_encode(json_buf , &key_data);

	cmp_softAP_tcp_send_data((uint8 *)json_buf, strlen(json_buf));
}
#endif



void start_app(void)
{
	int32 ret = 0;

	memset(&g_tinyusb_button_data, 0, sizeof(TINYUSB_BUTTON));
	memset(&g_gamepad_key, 0, sizeof(KEY_DETECTION));

	printf("\n\n-----------------ESP32 GAMEPAD----------------------\nV1.0.0 -HRAD V1.3\n\n");
	driver_init_gpio();

	ret = cmp_usb_init_gamepad();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}

#ifdef USER_TEST
	while (1) {
		delay_ms(1000);
		g_gamepad_key.KEY_A = BUTTON_DOWN;
		cmp_keyscan_dispose_gamepad_key(&g_gamepad_key, &g_tinyusb_button_data);
		cmp_usb_gamepad_send(&g_tinyusb_button_data);
		delay_ms(1000);
		g_gamepad_key.KEY_A = BUTTON_UP;
		cmp_keyscan_dispose_gamepad_key(&g_gamepad_key, &g_tinyusb_button_data);
		cmp_usb_gamepad_send(&g_tinyusb_button_data);
	}
#endif

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


