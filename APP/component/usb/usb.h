#ifndef CMP_USB_H
#define CMP_USB_H

#include "main_app.h"

typedef struct{
	int8_t x;
	int8_t y;
	int8_t z;
	int8_t rz;
	int8_t rx;
	int8_t ry;
	uint8_t hat;
	uint32_t buttons;
} TINYUSB_BUTTON;

typedef void (*cmp_usb_data_callback)(uint8 *recv_buffer, uint32 recv_len); // 用于在接收到数据时处理

int32 cmp_usb_init_gamepad(void);

int32 cmp_usb_gamepad_send(TINYUSB_BUTTON *button);

#endif

