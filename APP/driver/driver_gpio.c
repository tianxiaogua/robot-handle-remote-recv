/*
 * gpio.c
 *
 *  Created on: 2023年8月13日
 *      Author: tianxiaohua
 */

#include "driver_gpio.h"


void key_input_init(void)
{
	//设置GPIO为输入模式
	gpio_set_direction(GPIO_KEY,GPIO_MODE_INPUT);
}

int key_input_get(void)
{
	return gpio_get_level(GPIO_KEY);
}

void driver_init_gpio(void)
{
	/* LCD IO初始化*/
	gpio_set_direction(LCD_SPI_CS,GPIO_MODE_OUTPUT);
	GPIO_SetPin(LCD_SPI_CS,GPIO_LOW);
    gpio_set_direction(LCD_SPI_DC,GPIO_MODE_OUTPUT);
	GPIO_SetPin(LCD_SPI_DC,GPIO_HIGH);
	gpio_set_direction(GPIO_NUM_8, GPIO_MODE_INPUT); // 按键 CHA
	gpio_set_pull_mode(GPIO_NUM_8, GPIO_PULLDOWN_ONLY);

	/* 按钮IO初始化*/
	gpio_set_direction(GPIO_NUM_36,GPIO_MODE_OUTPUT); // 按键 UP
	gpio_set_pull_mode(GPIO_NUM_36, GPIO_PULLUP_ONLY);
	gpio_set_direction(GPIO_NUM_35,GPIO_MODE_OUTPUT); // 按键 DOWN
	gpio_set_pull_mode(GPIO_NUM_35, GPIO_PULLUP_ONLY);
	gpio_set_direction(GPIO_NUM_34,GPIO_MODE_OUTPUT); // 按键 LIFT
	gpio_set_pull_mode(GPIO_NUM_34, GPIO_PULLUP_ONLY);
	gpio_set_direction(GPIO_NUM_33,GPIO_MODE_OUTPUT); // 按键 RIGHT
	gpio_set_pull_mode(GPIO_NUM_33, GPIO_PULLUP_ONLY);

	gpio_set_direction(GPIO_NUM_1,GPIO_MODE_INPUT); // key_down1 扫描输入
	gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLDOWN_ONLY);
	gpio_set_direction(GPIO_NUM_14,GPIO_MODE_INPUT); // key_down2 按键输出
	gpio_set_pull_mode(GPIO_NUM_14, GPIO_PULLDOWN_ONLY);
	gpio_set_direction(GPIO_NUM_21,GPIO_MODE_INPUT); // key_down3 按键输出
	gpio_set_pull_mode(GPIO_NUM_21, GPIO_PULLDOWN_ONLY);
	GUA_LOGI("driver_init_gpio");
}


void GPIO_SetPin(gpio_num_t gpio_num, uint32_t level)
{
	gpio_set_level(gpio_num,level);
}

int32 GPIO_GetPin(gpio_num_t gpio_num)
{
	return gpio_get_level(gpio_num);
}
