#ifndef APP_TINYUSB_H
#define APP_TINYUSB_H

#include "main_app.h"

void app_tinyusb(void);
void app_tinyusb_serial(void);
int32 app_set_usb_mode(USB_OUT_CFG *usb_out_config);

#endif

