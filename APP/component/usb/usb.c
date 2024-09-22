/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"

#include "usb.h"
#include "driver_tool.h"


#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const uint8_t hid_report_descriptor[] = { // HID report descriptor
//    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD) ),
//    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE) ),
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(HID_USAGE_DESKTOP_GAMEPAD) )
};

static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};



TaskHandle_t Handle_usb_task = NULL;

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	(void) instance;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;
	GUA_LOGE("tud_hid_get_report_cb!!!!!!!!!!");
	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	GUA_LOGE("tud_hid_set_report_cb!!!!!!!!!!!!!!");
}

TU_ATTR_WEAK void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol)
{
	GUA_LOGE("tud_hid_set_protocol_cb!!!!!!!!!!!!!!!!");
}

TU_ATTR_WEAK bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate)
{
	GUA_LOGE("tud_hid_set_idle_cb!!!!!!!!!!!!!!!!!!!!!!");
	return 0;
}

TU_ATTR_WEAK void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
//	GUA_LOGI("tud_hid_report_complete_cb %d %s %d", instance, report, len);
}

static void tinyusb(void * pvParameters)
{
    GUA_LOGI("USB initialization");

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    GUA_LOGI("USB initialization DONE");

    while (1) {
        if (tud_mounted()) {
			GUA_LOGI("usb connected!");
			
		} else {
			GUA_LOGI("wait usb!");
		}
		delay_ms(2000);
    }
}

////////////////////////////////////////////外部接口///////////////////////////////////////////////////////////////////////////

int32 cmp_usb_init_gamepad(void)
{
	// 创建USB控制任务
	xTaskCreatePinnedToCore(tinyusb,            //任务函数
							"usb_task",          //任务名称
							4096,                //堆栈大小
							NULL,                //传递参数
							3,                   //任务优先级
							&Handle_usb_task,    //任务句柄
							tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
#ifdef DEBUG_TEST
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
	return REV_OK;
}

int32 cmp_usb_gamepad_send(TINYUSB_BUTTON *button)
{
	if (tud_mounted()) {
		tud_hid_gamepad_report(HID_USAGE_DESKTOP_GAMEPAD,
				button->x,
				button->y,
				button->z,
				button->rz,
				button->rx,
				button->ry,
				button->hat,
				button->buttons);
	} else {
//		GUA_LOGI("wait usb\n");
	}
	return REV_OK;
}
