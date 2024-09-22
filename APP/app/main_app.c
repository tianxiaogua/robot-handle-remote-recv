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

#define TCP_DATA_LEN 512
typedef struct
{
	uint8 tcp_data_buf[TCP_DATA_LEN];
	KEY_DETECTION key_value;
} APP_CONFIG;

static APP_CONFIG g_app_config;

TINYUSB_BUTTON g_button;
TINYUSB_BUTTON g_tinyusb_button_data;
KEY_DETECTION g_gamepad_key;

static char data_buf[128] = "012345678901234\n";
KEY_DETECTION key_data;
TaskHandle_t Handle_lCD_task = NULL;
TaskHandle_t Handle_gamepad_task = NULL;
// {"hd":"H3.1S1.0","v":{"U":1,"D":0,"L":0,"R":0,"A":1,"B":0,"X":0,"Y":0,"LB":0,"RB":0,"ST":0,"SE":0,"RK":0,"LK":0,"LX":127,"LY":127,"RX":127,"RY":127}}
// {"hd":"H3.1S1.0","v":{"U":1,"D":0,"L":0,"R":0,"A":1,"B":0,"X":0,"Y":0,"LB":0,"RB":0,"ST":0,"SE":0,"RK":0,"LK":0,"LX":127,"LY":127,"RX":127,"RY":127}}

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


static int32 app_decode_recv_data(uint8 *recv_buffer, uint32 recv_len)
{
	memset(data_buf, 0, sizeof(data_buf));

	GUA_LOGI("callback: data len:%d", recv_len);
	if (recv_len > 256) {
		GUA_LOGE("recv data length is too long!");
		return REV_ERR;
	}
	strcpy(data_buf, (char *)recv_buffer);
	protocol_decode(data_buf, &key_data);
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

// #define SOFT_FRO_GAMEPAD_RECV
#ifdef SOFT_FRO_GAMEPAD_RECV
/**
 * 以下为接收器部分的代码
 * */


static int32 app_wifi_event_handle_cb(int32 evint_handle)
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
	GUA_LOGI("recv!");
}

static void app_keyscan_cb(KEY_DETECTION *key_value)
{	
	cmp_keyscan_dispose_gamepad_key(key_value, &g_tinyusb_button_data); // 把原始按键数据转换成usb手柄数据

	cmp_usb_gamepad_send(&g_tinyusb_button_data); // 通过usb把游戏手柄数据发送出去

	memcpy(&g_app_config.key_value, key_value, sizeof(KEY_DETECTION));
}

static void gamepad_task(void * pvParameters)
{
	while (1) {
		memset(data_buf, 0, sizeof(data_buf));
		protocol_encode(data_buf , &g_app_config.key_value); // 把原始按键数据转换成json协议
		cmp_softAP_tcp_send_data((uint8 *)data_buf, strlen(data_buf)); // 通过网络发送json数据
		delay_ms(5);
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

	memset(&g_tinyusb_button_data, 0, sizeof(TINYUSB_BUTTON));
	memset(&g_gamepad_key, 0, sizeof(KEY_DETECTION));

	printf("\n\n-----------------ESP32 GAMEPAD----------------------\nV1.0.0 -HRAD V1.3\n\n");
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
							1,                   //任务优先级
							&Handle_lCD_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上

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
	cmp_keyscan_init();
	cmp_keyscan_sand_data_register(app_keyscan_cb, 5);

	ret = cmp_softAP_init();
	if (ret != REV_OK) {
		GUA_LOGE("err");
	}
	cmp_softAP_tcp_init_server();
	cmp_softAP_tcp_register_recv(softAP_recv_data_callback);

	// 创建无线数据发送任务
	xTaskCreatePinnedToCore(gamepad_task,            //任务函数
							"gamepad_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							1,                   //任务优先级
							&Handle_gamepad_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
#endif
	while (1) {
		delay_ms(100000);
	}
}


