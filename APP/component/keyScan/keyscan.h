#ifndef CPM_KEYSCAN_H
#define CPM_KEYSCAN_H

#include "main_app.h"
#include "usb.h"

#define BUTTON_DOWN 1 // 按钮按下 button down
#define BUTTON_UP 0 // 按钮按下 button down

typedef struct key_detection {
	uint8 KEY_CHA;

	uint8 KEY_LEFT;
	uint8 KEY_RIGHT;
	uint8 KEY_UP_;
	uint8 KEY_DOWN_;

	uint8 KEY_A;
	uint8 KEY_B;
	uint8 KEY_X;
	uint8 KEY_Y;

	uint8 KEY_LB;
	uint8 KEY_RB;

	uint8 KEY_SELECT;
	uint8 KEY_START;

	uint8 KEY_R_KEY;
	uint8 KEY_L_KEY;

	uint16 LX;
	uint16 LY;
	uint16 RX;
	uint16 RY;

	uint16 LT;
	uint16 RT;

	uint16 ca_calue_LX;
	uint16 ca_calue_LY;
	uint16 ca_calue_RX;
	uint16 ca_calue_RY;

} KEY_DETECTION;

typedef void (*cmp_keyscan_callback)(KEY_DETECTION *key_value); // 用于在接收到数据时处理

int32 cmp_keyscan_init(void);
void cmp_keyscan_dispose_gamepad_key(KEY_DETECTION *gamepad_key, TINYUSB_BUTTON *g_tinyusb_button_data);
int32 cmp_keyscan_sand_data_register(cmp_keyscan_callback fun, uint32 time);

#endif

