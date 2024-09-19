#ifndef CPM_KEYSCAN_H
#define CPM_KEYSCAN_H

#include "main_app.h"
#include "usb.h"
#include "protocol.h"

#define BUTTON_DOWN 1 // 按钮按下 button down
#define BUTTON_UP 0 // 按钮按下 button down

int32 cmp_keyscan_init(TINYUSB_BUTTON *g_tinyusb_button_data);
void cmp_keyscan_dispose_gamepad_key(KEY_DETECTION *gamepad_key, TINYUSB_BUTTON *g_tinyusb_button_data);

#endif

