#ifndef driver_adc_H_
#define driver_adc_H_

#include "driver/adc.h"

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
//#include "esp_adc_cal.h"

#include "driver_tool.h"

#define ADC1_CHAN1          ADC_CHANNEL_1
#define ADC1_CHAN2          ADC_CHANNEL_2
#define ADC1_CHAN3          ADC_CHANNEL_3
#define ADC1_CHAN4          ADC_CHANNEL_4
#define ADC1_CHAN5          ADC_CHANNEL_5

void driver_init_ADC(void);
uint32 get_adc_data(uint8 ADC_CHANNEL);

#endif /* APP_USER_TOOL_H_ */
