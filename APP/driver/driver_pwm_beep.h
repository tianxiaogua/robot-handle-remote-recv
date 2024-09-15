/*
 * led.h
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */

#ifndef DRIVER_PWM_BEEP
#define DRIVER_PWM_BEEP

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"


void init_beep(void);
void play_beep_ding(void);

#endif /* MAIN_LED_H_ */
