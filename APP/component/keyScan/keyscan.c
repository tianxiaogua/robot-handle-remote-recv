#include <stdlib.h>
#include "stdio.h"
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "class/hid/hid_device.h"

#include "tinyusb.h"
#include "pca9557.h"

#include "driver_tool.h"
#include "driver/gpio.h"
#include "driver_nvs.h"

#include "keyscan.h"
#include "protocol.h"
#include "usb.h"

/* 行输出 */
#define KEY_ROW_1 GPIO_NUM_36
#define KEY_ROW_2 GPIO_NUM_35
#define KEY_ROW_3 GPIO_NUM_34
#define KEY_ROW_4 GPIO_NUM_33
/* 列输入 */
#define KEY_LIN_1 GPIO_NUM_1
#define KEY_LIN_2 GPIO_NUM_14
#define KEY_LIN_3 GPIO_NUM_21
/* 行输出 */
#define KEY_ROW_1_HIG GPIO_SetPin(KEY_ROW_1, GPIO_HIGH)
#define KEY_ROW_1_LOW GPIO_SetPin(KEY_ROW_1, GPIO_LOW)
#define KEY_ROW_2_HIG GPIO_SetPin(KEY_ROW_2, GPIO_HIGH)
#define KEY_ROW_2_LOW GPIO_SetPin(KEY_ROW_2, GPIO_LOW)
#define KEY_ROW_3_HIG GPIO_SetPin(KEY_ROW_3, GPIO_HIGH)
#define KEY_ROW_3_LOW GPIO_SetPin(KEY_ROW_3, GPIO_LOW)
#define KEY_ROW_4_HIG GPIO_SetPin(KEY_ROW_4, GPIO_HIGH)
#define KEY_ROW_4_LOW GPIO_SetPin(KEY_ROW_4, GPIO_LOW)
/* 列输入 */
#define KEY_LIN_1_READ GPIO_GetPin(KEY_LIN_1)
#define KEY_LIN_2_READ GPIO_GetPin(KEY_LIN_2)
#define KEY_LIN_3_READ GPIO_GetPin(KEY_LIN_3)

#define ECC_VALUE 15	   // 偏心值 Eccentricity Value
#define SUBDIV_VALUE 128   // 划分数值
#define MAX_ECC_VALUE 4095 // ADC最大划分值
#define MEDIA_VALE 127	   // 摇杆中间值 Median
#define MAX_VALE 256	   // 摇杆中间值 Median

// 模拟引脚输入
typedef struct nvs_gamepad_
{
	nvs_handle nvs_handler;
	char namespace[32];
	char lift_stick_x[32]; // 左摇杆数值x
	uint32_t lift_stick_x_value;
	char lift_stick_y[32]; // 左摇杆数值y
	uint32_t lift_stick_y_value;
	char right_stick_x[32]; // 右摇杆数值x
	uint32_t right_stick_x_value;
	char right_stick_y[32]; // 右摇杆数值y
	uint32_t right_stick_y_value;
} NVS_GAMEPAD_TYPE;

NVS_GAMEPAD_TYPE nvs_gamepad = {
	.namespace = "key_list",
	.lift_stick_x = "lift_stick_x",
	.lift_stick_x_value = 0,
	.lift_stick_y = "lift_stick_y",
	.lift_stick_y_value = 0,
	.right_stick_x = "right_stick_x",
	.right_stick_x_value = 0,
	.right_stick_y = "right_stick_y",
	.right_stick_y_value = 0};

typedef struct
{
	uint16_t shutdown_time; // 关机
	uint16_t time_flag;
} KEY_SHUTDOWN_CFG;

typedef struct
{
	/* data */
	cmp_keyscan_callback keyscan_cb;
	KEY_DETECTION key_value;
	uint16 ca_calue_LX;
	uint16 ca_calue_LY;
	uint16 ca_calue_RX;
	uint16 ca_calue_RY;
	uint32 time;
} KEYSCAN_CONFIG;

KEY_SHUTDOWN_CFG g_key_shutdown_cfg; // 控制关机
TaskHandle_t Handle_key_task = NULL;
KEYSCAN_CONFIG g_keyscan_cfg;

static void key_scan(uint8 *key_value)
{
	// 四行三列扫描键盘
	KEY_ROW_1_HIG;
	KEY_ROW_2_LOW;
	KEY_ROW_3_LOW;
	KEY_ROW_4_LOW;
	key_value[0] = KEY_LIN_1_READ;
	key_value[1] = KEY_LIN_2_READ;
	key_value[2] = KEY_LIN_3_READ;
	delay_ms(2);

	KEY_ROW_1_LOW;
	KEY_ROW_2_HIG;
	KEY_ROW_3_LOW;
	KEY_ROW_4_LOW;
	key_value[3] = KEY_LIN_1_READ;
	key_value[4] = KEY_LIN_2_READ;
	key_value[5] = KEY_LIN_3_READ;
	delay_ms(2);

	KEY_ROW_1_LOW;
	KEY_ROW_2_LOW;
	KEY_ROW_3_HIG;
	KEY_ROW_4_LOW;
	key_value[6] = KEY_LIN_1_READ;
	key_value[7] = KEY_LIN_2_READ;
	key_value[8] = KEY_LIN_3_READ;
	delay_ms(2);

	KEY_ROW_1_LOW;
	KEY_ROW_2_LOW;
	KEY_ROW_3_LOW;
	KEY_ROW_4_HIG;
	key_value[9] = KEY_LIN_1_READ;
	key_value[10] = KEY_LIN_2_READ;
	key_value[11] = KEY_LIN_3_READ;
}

// 校准摇杆
static int32 app_calibration_rocker(void)
{
	int32 temp_value[4] = {0};

	driver_nvs_get_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.lift_stick_x, &nvs_gamepad.lift_stick_x_value);
	driver_nvs_get_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.lift_stick_y, &nvs_gamepad.lift_stick_y_value);
	driver_nvs_get_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.right_stick_x, &nvs_gamepad.right_stick_x_value);
	driver_nvs_get_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.right_stick_y, &nvs_gamepad.right_stick_y_value);
	g_keyscan_cfg.ca_calue_LX = nvs_gamepad.lift_stick_x_value;
	g_keyscan_cfg.ca_calue_LY = nvs_gamepad.lift_stick_y_value;
	g_keyscan_cfg.ca_calue_RX = nvs_gamepad.right_stick_x_value;
	g_keyscan_cfg.ca_calue_RY = nvs_gamepad.right_stick_y_value;
	GUA_LOGI("read ca_calue_LX value:%d", g_keyscan_cfg.ca_calue_LX);
	GUA_LOGI("read ca_calue_LY value:%d", g_keyscan_cfg.ca_calue_LY);
	GUA_LOGI("read ca_calue_RX value:%d", g_keyscan_cfg.ca_calue_RX);
	GUA_LOGI("read ca_calue_RY value:%d", g_keyscan_cfg.ca_calue_RY);

	nvs_gamepad.lift_stick_x_value = 0;
	if (nvs_gamepad.lift_stick_x_value == 0)
	{ // || nvs_gamepad.lift_stick_y_value == 0 || nvs_gamepad.right_stick_x_value == 0 || nvs_gamepad.right_stick_y_value == 0) { // 没有校准数据，重新校准摇杆
		for (uint32_t i = 0; i < 10; i++)
		{
			temp_value[0] += get_adc_data(ADC_CHANNEL_3); // LX
			temp_value[1] += get_adc_data(ADC_CHANNEL_4); // LY
			temp_value[2] += get_adc_data(ADC_CHANNEL_2); // RX
			temp_value[3] += get_adc_data(ADC_CHANNEL_1); // RY
			// GUA_LOGI("get calibration rocker:%d %d %d %d", temp_value[0], temp_value[1], temp_value[2], temp_value[3]);
			delay_ms(1);
		}
		GUA_LOGW("get average calibration rocker:%d %d %d %d", temp_value[0] / 10, temp_value[1] / 10, temp_value[2] / 10, temp_value[3] / 10);
		g_keyscan_cfg.ca_calue_LX = temp_value[0] / 10;
		g_keyscan_cfg.ca_calue_LY = temp_value[1] / 10;
		g_keyscan_cfg.ca_calue_RX = temp_value[2] / 10;
		g_keyscan_cfg.ca_calue_RY = temp_value[3] / 10;
		driver_nvs_set_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.lift_stick_x, temp_value[0] / 10);
		driver_nvs_set_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.lift_stick_y, temp_value[1] / 10);
		driver_nvs_set_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.right_stick_x, temp_value[2] / 10);
		driver_nvs_set_u32(nvs_gamepad.nvs_handler, nvs_gamepad.namespace, nvs_gamepad.right_stick_y, temp_value[3] / 10);
	}
	else
	{
		GUA_LOGI("calibration rocker is ok :%d %d %d %d\n", temp_value[0], temp_value[1], temp_value[2], temp_value[3]);
	}

	return REV_OK;
}

static uint16 get_stick_value(uint16 stick, uint16 media_stick, uint16 filter_channer)
{
	uint16 temp_ecc_value = 0;
	uint16 temp_final_value = 0;

	stick = driver_kalman_filter(filter_channer, stick);
	if (stick > (media_stick - ECC_VALUE) && stick < (media_stick + ECC_VALUE))
	{
		temp_final_value = 0;
	}
	else
	{
		if (stick < media_stick - ECC_VALUE)
		{ // 0 ~ 127
			temp_ecc_value = media_stick / 128;
			temp_final_value = 127 - stick / temp_ecc_value;
		}
		else
		{ // 128 ~256 最大值128
			temp_ecc_value = (4095 - media_stick) / 128;
			temp_final_value = 128 + (4095 - stick) / temp_ecc_value;
		}
	}
	if (temp_final_value > 6000)
	{
		temp_final_value = 0;
	}
	return temp_final_value;
}

static void keyboard_scan(void)
{
	uint8 key_value[10] = {0};

	key_scan(key_value);
	g_keyscan_cfg.key_value.KEY_UP_ = key_value[0];
	g_keyscan_cfg.key_value.KEY_DOWN_ = key_value[3];
	g_keyscan_cfg.key_value.KEY_LEFT = key_value[6];
	g_keyscan_cfg.key_value.KEY_RIGHT = key_value[9];
	g_keyscan_cfg.key_value.KEY_A = key_value[1];
	g_keyscan_cfg.key_value.KEY_B = key_value[4];
	g_keyscan_cfg.key_value.KEY_X = key_value[7];
	g_keyscan_cfg.key_value.KEY_Y = key_value[10];
	g_keyscan_cfg.key_value.KEY_LB = key_value[5];
	g_keyscan_cfg.key_value.KEY_RB = key_value[2];
	g_keyscan_cfg.key_value.LT = key_value[11];
	g_keyscan_cfg.key_value.RT = key_value[8];

	updata_pca9557_io_input();
	g_keyscan_cfg.key_value.KEY_SELECT = !read_pca9557_io_input(1);
	g_keyscan_cfg.key_value.KEY_START = !read_pca9557_io_input(7);

	g_keyscan_cfg.key_value.KEY_R_KEY = !read_pca9557_io_input(2);
	g_keyscan_cfg.key_value.KEY_L_KEY = !read_pca9557_io_input(3);

	g_keyscan_cfg.key_value.KEY_CHA = !GPIO_GetPin(KEY_CHA_NUM);

	g_keyscan_cfg.key_value.LX = get_adc_data(ADC_CHANNEL_3);
	g_keyscan_cfg.key_value.LY = get_adc_data(ADC_CHANNEL_4);
	g_keyscan_cfg.key_value.RX = get_adc_data(ADC_CHANNEL_2);
	g_keyscan_cfg.key_value.RY = get_adc_data(ADC_CHANNEL_1);
	//	get_adc_data(ADC_CHANNEL_5);

	if (g_keyscan_cfg.key_value.KEY_LEFT == 1 && g_keyscan_cfg.key_value.KEY_B == 1)
	{										// 按下的时候是0
		g_key_shutdown_cfg.shutdown_time++; // 每次循环约10毫秒 长按3秒关机，相当于循环约290次
		if (g_key_shutdown_cfg.shutdown_time > 200)
		{
			GUA_LOGW("shutdown!\n");
			set_pca9557_io_low(6); // 用于关机的IO
			delay_ms(10000);
		}
	}
	else
	{
		g_key_shutdown_cfg.shutdown_time = 0;
	}
	// g_key_shutdown_cfg.time_flag++;
	// if (g_key_shutdown_cfg.time_flag >5) {
	// 	g_key_shutdown_cfg.time_flag = 0;
	// 	printf("CHA:%d UP:%d DOWN:%d LEFT:%d RIGHT:%d A:%d B:%d X:%d Y:%d LB:%d RB:%d LT:%d RT:%d START:%d SELECT:%d R_KEY:%d L_KEY:%d  %d %d %d %d\r\n",
	// 		g_keyscan_cfg.key_value.KEY_CHA,
	// 		g_keyscan_cfg.key_value.KEY_UP_,
	// 		g_keyscan_cfg.key_value.KEY_DOWN_,
	// 		g_keyscan_cfg.key_value.KEY_LEFT,
	// 		g_keyscan_cfg.key_value.KEY_RIGHT,
	// 		g_keyscan_cfg.key_value.KEY_A,
	// 		g_keyscan_cfg.key_value.KEY_B,
	// 		g_keyscan_cfg.key_value.KEY_X,
	// 		g_keyscan_cfg.key_value.KEY_Y,
	// 		g_keyscan_cfg.key_value.KEY_LB,
	// 		g_keyscan_cfg.key_value.KEY_RB,
	// 		g_keyscan_cfg.key_value.LT,
	// 		g_keyscan_cfg.key_value.RT,
	// 		g_keyscan_cfg.key_value.KEY_START,
	// 		g_keyscan_cfg.key_value.KEY_SELECT,
	// 		g_keyscan_cfg.key_value.KEY_R_KEY,
	// 		g_keyscan_cfg.key_value.KEY_L_KEY,
	// 		g_keyscan_cfg.key_value.LX,
	// 		g_keyscan_cfg.key_value.LY,
	// 		g_keyscan_cfg.key_value.RX,
	// 		g_keyscan_cfg.key_value.RY
	// 		);
	// }
}
static void keyboard_scan_handle(void *pvParameters)
{
	uint8 key_value[10] = {0};

	while (1)
	{
		keyboard_scan();
		if (g_keyscan_cfg.keyscan_cb != NULL)
		{
			g_keyscan_cfg.keyscan_cb(&g_keyscan_cfg.key_value);
		}
		delay_ms(g_keyscan_cfg.time);
	}
}

//////////////////////////////////////////以下外部接口/////////////////////////////////////////////////////////////////////////////////////////

void cmp_keyscan_dispose_gamepad_key(KEY_DETECTION *gamepad_key, TINYUSB_BUTTON *g_tinyusb_button_data)
{
	if (gamepad_key->KEY_A == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_A;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_A;
	}
	if (gamepad_key->KEY_B == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_B;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_B;
	}
	if (gamepad_key->KEY_X == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_X;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_X;
	}
	if (gamepad_key->KEY_Y == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_Y;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_Y;
	}
	if (gamepad_key->KEY_LB == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_TL2;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_TL2;
	}
	if (gamepad_key->KEY_RB == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_TR2;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_TR2;
	}
	if (gamepad_key->KEY_SELECT == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_SELECT;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_SELECT;
	}
	if (gamepad_key->KEY_START == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_START;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_START;
	}
	if (gamepad_key->KEY_R_KEY == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_THUMBR;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_THUMBR;
	}
	if (gamepad_key->KEY_L_KEY == BUTTON_DOWN)
	{
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_THUMBL;
	}
	else
	{
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_THUMBL;
	}

	if (gamepad_key->KEY_UP_ == BUTTON_DOWN)
	{
		if (gamepad_key->KEY_RIGHT == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_RIGHT;
		}
		else if (gamepad_key->KEY_LEFT == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_LEFT;
		}
		else
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP;
		}
	}
	else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN)
	{
		if (gamepad_key->KEY_RIGHT == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_RIGHT;
		}
		else if (gamepad_key->KEY_LEFT == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_LEFT;
		}
		else
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN;
		}
	}
	else if (gamepad_key->KEY_RIGHT == BUTTON_DOWN)
	{
		if (gamepad_key->KEY_UP_ == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_RIGHT;
		}
		else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_RIGHT;
		}
		else
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_RIGHT;
		}
	}
	else if (gamepad_key->KEY_LEFT == BUTTON_DOWN)
	{
		if (gamepad_key->KEY_UP_ == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_LEFT;
		}
		else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN)
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_LEFT;
		}
		else
		{
			g_tinyusb_button_data->hat = GAMEPAD_HAT_LEFT;
		}
	}
	else
	{
		g_tinyusb_button_data->hat = GAMEPAD_HAT_CENTERED;
	}

	g_tinyusb_button_data->x = get_stick_value(gamepad_key->RX, g_keyscan_cfg.ca_calue_RX, filter_channer1);
	g_tinyusb_button_data->y = get_stick_value(gamepad_key->RY, g_keyscan_cfg.ca_calue_RY, filter_channer2);
	g_tinyusb_button_data->rx = get_stick_value(gamepad_key->LX, g_keyscan_cfg.ca_calue_LX, filter_channer3);
	g_tinyusb_button_data->ry = get_stick_value(gamepad_key->LY, g_keyscan_cfg.ca_calue_LY, filter_channer4);
}

int32 cmp_keyscan_sand_data_register(cmp_keyscan_callback fun, uint32 time)
{
	g_keyscan_cfg.time = time;
	g_keyscan_cfg.keyscan_cb = fun;
	if (g_keyscan_cfg.keyscan_cb == NULL)
	{
		return REV_ERR;
	}
	return REV_OK;
}

int32 cmp_keyscan_init(void)
{

	int32 ret = 0;

	memset(&g_keyscan_cfg.key_value, 0, sizeof(g_keyscan_cfg.key_value));
	memset(&g_key_shutdown_cfg, 0, sizeof(g_key_shutdown_cfg));

	g_keyscan_cfg.keyscan_cb = NULL;

	ret = app_calibration_rocker();

	// 创建按键控制任务
	xTaskCreatePinnedToCore(keyboard_scan_handle,  // 任务函数
							"keyboard_task",	   // 任务名称
							4096+2048,				   // 堆栈大小
							NULL,				   // 传递参数
							2,					   // 任务优先级
							&Handle_key_task, // 任务句柄
							tskNO_AFFINITY);	   // 无关联，不绑定在任何一个核上

	return ret;
}
