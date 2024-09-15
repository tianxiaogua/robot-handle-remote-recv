/*
 * led.h
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */

#ifndef MAIN_LED_H_
#define MAIN_LED_H_

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"



void ledc_init(void);
void ledc_set(ledc_channel_t channel, uint32_t duty);

#endif /* MAIN_LED_H_ */
