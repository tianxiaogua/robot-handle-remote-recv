#include <stdlib.h>
#include "stdio.h"
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tinyusb.h"
#include "class/hid/hid_device.h"

#include "driver_tool.h"
#include "driver/gpio.h"

#include "keyscan.h"
#include "protocol.h"
#include "usb.h"


#define ECC_VALUE      15   // 偏心值 Eccentricity Value
#define SUBDIV_VALUE   128  // 划分数值
#define MAX_ECC_VALUE  4095 // ADC最大划分值
#define MEDIA_VALE     127  // 摇杆中间值 Median
#define MAX_VALE       256  // 摇杆中间值 Median

uint16 get_stick_value(uint16 stick, uint16 media_stick, uint16 filter_channer)
{
	uint16 temp_ecc_value = 0;
	uint16 temp_final_value = 0;

	stick = driver_kalman_filter(filter_channer, stick);
	if (stick > (media_stick - ECC_VALUE) && stick < (media_stick + ECC_VALUE)) {
		temp_final_value = 0;
	}
	else {
		if (stick < media_stick - ECC_VALUE) { // 0 ~ 127
			temp_ecc_value = media_stick  / 128;
			temp_final_value = 127 - stick / temp_ecc_value;
		} else { // 128 ~256 最大值128
			temp_ecc_value = (4095 - media_stick) / 128;
			temp_final_value = 128 + (4095 - stick) / temp_ecc_value;
		}
	}
	if (temp_final_value > 6000) {
		temp_final_value = 0;
	}
	return temp_final_value;
}


void cmp_keyscan_dispose_gamepad_key(KEY_DETECTION *gamepad_key, TINYUSB_BUTTON *g_tinyusb_button_data)
{
	if (gamepad_key->KEY_A == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_A;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_A;
	}
	if (gamepad_key->KEY_B == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_B;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_B;
	}
	if (gamepad_key->KEY_X == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_X;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_X;
	}
	if (gamepad_key->KEY_Y == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_Y;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_Y;
	}
	if (gamepad_key->KEY_LB == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_TL2;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_TL2;
	}
	if (gamepad_key->KEY_RB == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_TR2;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_TR2;
	}
	if (gamepad_key->KEY_SELECT == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_SELECT;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_SELECT;
	}
	if (gamepad_key->KEY_START == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_START;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_START;
	}
	if (gamepad_key->KEY_R_KEY == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_THUMBR;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_THUMBR;
	}
	if (gamepad_key->KEY_L_KEY == BUTTON_DOWN) {
		g_tinyusb_button_data->buttons |= GAMEPAD_BUTTON_THUMBL;
	} else {
		g_tinyusb_button_data->buttons &= ~GAMEPAD_BUTTON_THUMBL;
	}

	if (gamepad_key->KEY_UP_ == BUTTON_DOWN) {
		if(gamepad_key->KEY_RIGHT == BUTTON_DOWN) {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_RIGHT;
		} else if (gamepad_key->KEY_LEFT == BUTTON_DOWN){
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_LEFT;
		} else {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP;
		}
	} else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN) {
		if(gamepad_key->KEY_RIGHT == BUTTON_DOWN) {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_RIGHT;
		} else if (gamepad_key->KEY_LEFT == BUTTON_DOWN){
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_LEFT;
		} else {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN;
		}
	} else if (gamepad_key->KEY_RIGHT == BUTTON_DOWN) {
		if(gamepad_key->KEY_UP_ == BUTTON_DOWN) {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_RIGHT;
		} else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN){
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_RIGHT;
		} else {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_RIGHT;
		}
	} else if (gamepad_key->KEY_LEFT == BUTTON_DOWN) {
		if(gamepad_key->KEY_UP_ == BUTTON_DOWN) {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_UP_LEFT;
		} else if (gamepad_key->KEY_DOWN_ == BUTTON_DOWN){
			g_tinyusb_button_data->hat = GAMEPAD_HAT_DOWN_LEFT;
		} else {
			g_tinyusb_button_data->hat = GAMEPAD_HAT_LEFT;
		}
	} else {
		g_tinyusb_button_data->hat = GAMEPAD_HAT_CENTERED;
	}

	g_tinyusb_button_data->x = get_stick_value(gamepad_key->RX, gamepad_key->ca_calue_RX, filter_channer1);
	g_tinyusb_button_data->y = get_stick_value(gamepad_key->RY, gamepad_key->ca_calue_RY, filter_channer2);
	g_tinyusb_button_data->rx = get_stick_value(gamepad_key->LX, gamepad_key->ca_calue_LX, filter_channer3);
	g_tinyusb_button_data->ry = get_stick_value(gamepad_key->LY, gamepad_key->ca_calue_LY, filter_channer4);
}


int32 cmp_keyscan_init(TINYUSB_BUTTON *g_tinyusb_button_data)
{
	memset(g_tinyusb_button_data, 0, sizeof(TINYUSB_BUTTON));
	return REV_OK;
}
