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
#include "main_app.h"
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
// LCD
#include "lcd.h"
#include "lcd_init.h"
//#include "pic.h"
//// 设备驱动
//#include "inv_mpu.h"
//#include "inv_mpu_dmp_motion_driver.h"
//#include "mpu6050.h"

// 应用
#include "protocol.h"
#include "main_app.h"

#if 0
#define SOFT_FRO_GAMEPAD_RECV
#endif

#define PROTOCOL_DATA_HEAD_1 0x5e// 协议数据头
#define PROTOCOL_DATA_HEAD_2 0x55// 协议数据头
#define PROTOCOL_DATA_HEAD_3 0x5e// 协议数据头
#define PROTOCOL_DATA_END_1 0x0d// 协议数据头
#define PROTOCOL_DATA_END_2 0x0a// 协议数据头
#define DATA_BUF_LEN 32
#define PROTOCOL_PAYLOAD_LEN DATA_BUF_LEN-6
#define INTERVAL_TIME 2// 无线发送时间间隔 interval
#define SOCKET_DATA_LEN 512

typedef struct
{
	uint8 tcp_data_buf[SOCKET_DATA_LEN];
	KEY_DETECTION key_value;
} APP_CONFIG;

static APP_CONFIG g_app_config;

TINYUSB_BUTTON g_button;
TINYUSB_BUTTON g_tinyusb_button_data;
KEY_DETECTION g_gamepad_key;


KEY_DETECTION key_data;
TaskHandle_t Handle_lCD_task = NULL;
TaskHandle_t Handle_gamepad_task = NULL;

/* 开机 */
void muc_starting_up(void)
{
	uint8_t KEY_START;
	uint8_t key_time = 0;
	init_pca9557();
	KEY_START  = read_pca9557_io_input(7);
	GUA_LOGI("KEY_START:%d",KEY_START);
	while(KEY_START == 0) { // 按键按下的时候是0
		delay_ms(100); // 延时100ms消抖
		KEY_START  = read_pca9557_io_input(7);
		key_time++;
		if(key_time >2){ // 超过2秒开机
			set_pca9557_io_high(6);
			GUA_LOGI("start muc");
			return;
		}
	}
	GUA_LOGE("start muc fause");
}

// 数据组包 数据负载
int32 app_epd_gamepad(uint8 *data_payload, uint32 data_len, uint8 *out_buf, uint32 out_buf_len)
{
	if (out_buf_len < DATA_BUF_LEN) {
		GUA_LOGE("input buf_len err");
		return REV_ERR;
	}

	if (data_len > PROTOCOL_PAYLOAD_LEN) {
		GUA_LOGE("input data_payload err");
		return REV_ERR;
	}

	memset(out_buf, 0, out_buf_len);
	out_buf[0] = PROTOCOL_DATA_HEAD_1;
	out_buf[1] = PROTOCOL_DATA_HEAD_2;
	out_buf[2] = PROTOCOL_DATA_HEAD_3;
	out_buf[3] = PROTOCOL_PAYLOAD_LEN;
	for (u32 i = 0; i < data_len; i++)
	{
		out_buf[i+4] = data_payload[i];
	}

	out_buf[out_buf_len-2] = PROTOCOL_DATA_END_1;
	out_buf[out_buf_len-1] = PROTOCOL_DATA_END_2;
	return REV_OK;
}

int32 app_decode_recv_data(uint8 *recv_buffer, uint32 recv_len)
{
	uint8 data_buf[PROTOCOL_PAYLOAD_LEN] = {0};
	uint32 i = 0;

	if (recv_len < DATA_BUF_LEN) {
		GUA_LOGE("recv len err!");
		return REV_ERR;
	}
//	GUA_LOGI("callback: data len:%d", recv_len);

	if (recv_buffer[0] == PROTOCOL_DATA_HEAD_1 && recv_buffer[1] == PROTOCOL_DATA_HEAD_2 && recv_buffer[2] == PROTOCOL_DATA_HEAD_3) {
		if (recv_buffer[3] != PROTOCOL_PAYLOAD_LEN) {
			GUA_LOGE("recv payload err!");
			return REV_ERR;
		}
		for (; i < PROTOCOL_PAYLOAD_LEN; i++) {
//			if (recv_buffer[i+4] == 0) {
//				break;
//			}
			data_buf[i] = recv_buffer[i+4];
//			printf("%02x ",data_buf[i]);
		}
	} else {
		GUA_LOGE("recv head err!");
		return REV_ERR;
	}

	app_protocol_decode_protoc(data_buf, i, &key_data);
//	printf("UP:%d DOWN:%d LEFT:%d RIGHT:%d A:%d B:%d X:%d Y:%d LB:%d RB:%d START:%d SELECT:%d R_KEY:%d L_KEY:%d  %d %d %d %d\r\n",
//		key_data.KEY_UP_,
//		key_data.KEY_DOWN_,
//		key_data.KEY_LEFT,
//		key_data.KEY_RIGHT,
//		key_data.KEY_A,
//		key_data.KEY_B,
//		key_data.KEY_X,
//		key_data.KEY_Y,
//		key_data.KEY_LB,
//		key_data.KEY_RB,
//		key_data.KEY_START,
//		key_data.KEY_SELECT,
//		key_data.KEY_R_KEY,
//		key_data.KEY_L_KEY,
//		key_data.LX,
//		key_data.LY,
//		key_data.RX,
//		key_data.RY
//	);
	return REV_OK;
}


#ifdef SOFT_FRO_GAMEPAD_RECV
/**
 * 以下为接收器部分的代码
 * */


static int32 app_wifi_event_handle_cb(int32 evint_handle)
{
	int32 ret = 0;

	switch (evint_handle) {
	case WIFI_CONNECTED:
		ret = cmp_wifi_socket_client_init();
		break;
	case WIFI_TCP_RECV_DATA:
		memset(g_app_config.tcp_data_buf, 0, SOCKET_DATA_LEN);
		ret = cmp_wifi_recv_data(g_app_config.tcp_data_buf, SOCKET_DATA_LEN);
		if (ret != REV_ERR) { // 数据正确的时候处理数据
			app_decode_recv_data(g_app_config.tcp_data_buf, ret); // 把接收到的数据解码为按键数据
			cmp_keyscan_dispose_gamepad_key(&key_data, &g_tinyusb_button_data); // 将按键数据处理成为游戏手柄数据
			cmp_usb_gamepad_send(&g_tinyusb_button_data); // 通过usb把游戏手柄数据发送出去
//			GUA_LOGI("len:%d recv:%s", ret, g_app_config.tcp_data_buf);
//			GUA_LOGI("??????");
//			cmp_wifi_send_data(g_app_config.tcp_data_buf, ret); // 测试代码
		}
		break;
	case WIFI_TCP_DISCONNECTED:
		cmp_wifi_client_deinit();
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
	GUA_LOGI("recv:%s", recv_buffer);
	// cmp_softAP_send_data(recv_buffer, recv_len);
}

static void app_keyscan_cb(KEY_DETECTION *key_value)
{	
	cmp_keyscan_dispose_gamepad_key(key_value, &g_tinyusb_button_data); // 把原始按键数据转换成usb手柄数据

	cmp_usb_gamepad_send(&g_tinyusb_button_data); // 通过usb把游戏手柄数据发送出去

	memcpy(&g_app_config.key_value, key_value, sizeof(KEY_DETECTION));
}

static void gamepad_task(void * pvParameters)
{
	int32 ret = 0;
	uint8 data_buf[SOCKET_DATA_LEN];
	memset(data_buf, 0, DATA_BUF_LEN);
	while (1) {
		ret  = cmp_softAP_get_connect_status();
		if (ret == CMP_SOFTAP_TCP_CONNECTED) {
			uint8 payload[PROTOCOL_PAYLOAD_LEN];
			app_protocol_encode_protoc(&g_app_config.key_value, payload, PROTOCOL_PAYLOAD_LEN); // 把原始按键数据转换成protoc协议
			app_epd_gamepad(payload, PROTOCOL_PAYLOAD_LEN, data_buf, DATA_BUF_LEN);
			cmp_softAP_send_data(data_buf, DATA_BUF_LEN); // 通过网络发送数据
			delay_ms(10);
		} else {
			delay_ms(2000);
			GUA_LOGI("wait gamepad rev...");
		}
	}
}
#endif



void lcd_task(void * pvParameters)
{
	while(1)
	{
		LCD_Fill(0,0,LCD_W,LCD_H,GREEN);
		delay_ms(1000);
		LCD_Fill(0,0,LCD_W,LCD_H,RED);
		delay_ms(1000);
	}
}

void start_app(void)
{
	int32 ret = 0;

	printf("\n\n-----------------ESP32 GAMEPAD----------------------\nV1.0.0 -HRAD V1.3\n\n");

#ifndef SOFT_FRO_GAMEPAD_RECV // 软件版本设置为接收器
	muc_starting_up(); // 开机过程
	init_beep();
	play_beep_ding();
	driver_init_ADC();
	driver_init_gpio();
	spi2_init();
	LCD_Init();//LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,0xFFFF);

//	// 对MPU6050进行测试
//	GUA_LOGI("start mpu init");
//	ret = MPU_Init();
//	if (ret != REV_OK){ //初始化
//		GUA_LOGE("mpu init error!");
//	}
//	GUA_LOGI("start mpu dmp lib init");
//	while(mpu_dmp_init()) {//初始化 MPU6050的DMP
//		while(1) {
//			GUA_LOGE("error");
//			delay_ms(500);
//		}
//	}
//	GUA_LOGI("init mpu 6050 done");

	// 创建LCD控制任务
	xTaskCreatePinnedToCore(lcd_task,            //任务函数
							"lcd_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							2,                   //任务优先级
							&Handle_lCD_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
#endif

	memset(&g_tinyusb_button_data, 0, sizeof(TINYUSB_BUTTON));
	memset(&g_gamepad_key, 0, sizeof(KEY_DETECTION));
	ret = cmp_usb_init_gamepad();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}

#ifdef SOFT_FRO_GAMEPAD_RECV // 软件版本设置为接收器
	cmp_wifi_init_sta();
	cmp_wifi_event_handle_register(app_wifi_event_handle_cb);
#else // 遥控器部分代码
	cmp_keyscan_init();
	cmp_keyscan_sand_data_register(app_keyscan_cb, INTERVAL_TIME);

	ret = cmp_softAP_init();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}
	cmp_softAP_init_server();
	cmp_softAP_register_recv(softAP_recv_data_callback);

	// 创建无线数据发送任务
	xTaskCreatePinnedToCore(gamepad_task,            //任务函数
							"gamepad_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							0,                   //任务优先级
							&Handle_gamepad_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
#endif
	while (1) {
		delay_ms(2500);
		GUA_LOGW("free_heap_size = %ld\n", esp_get_free_heap_size());
	}
}


