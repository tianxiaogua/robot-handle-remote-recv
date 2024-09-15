/*
 * gpio.h
 *
 *  Created on: 2023年8月13日
 *      Author: tianxiaohua
 */

#ifndef APP_GPIO_GPIO_H_
#define APP_GPIO_GPIO_H_

#include "driver/gpio.h"
#include "driver_tool.h"

#define KEY_DEWN 0 // 按键被按下
#define KEY_UP   1 // 按键抬起

// level Output level. 0: low ; 1: high
#define GPIO_LOW 0
#define GPIO_HIGH 1

#define LCD_SPI_CS   GPIO_NUM_10
#define LCD_SPI_DC   GPIO_NUM_45
#define KEY_CHA_NUM   GPIO_NUM_8

#define GPIO_KEY   0

void key_input_init(void);

int key_input_get(void);

void driver_init_gpio(void);

void GPIO_SetPin(gpio_num_t gpio_num, uint32_t level);
int32 GPIO_GetPin(gpio_num_t gpio_num);

#endif /* APP_GPIO_GPIO_H_ */
