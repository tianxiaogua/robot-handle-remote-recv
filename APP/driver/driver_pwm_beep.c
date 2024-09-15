/*
 * led.c
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */

#include "driver_pwm_beep.h"
#include "driver_tool.h"

#define BEEP_GPIO GPIO_NUM_17

void init_beep(void)
{
	int32 recv = 0;
	// 设置LEDC周边配置
	// 准备并应用LEDC PWM定时器配置
	ledc_timer_config_t ledc_timer1 = {
		.speed_mode       = LEDC_LOW_SPEED_MODE,
		.timer_num        = LEDC_TIMER_0,
		.duty_resolution  = LEDC_TIMER_13_BIT, // 设置分辨率为13位
		.freq_hz          = 1000,  // 频率单位为赫兹。频率设定为5千赫
		.clk_cfg          = LEDC_AUTO_CLK
	};
	recv = ledc_timer_config(&ledc_timer1);
	if (recv != ESP_OK) {
		GUA_LOGE("LED timer config ERROR!");
	}

	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel1 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_0,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE, // 禁用ＬＥＤＣ中断
		.gpio_num       = BEEP_GPIO,  // 绑定GPIO
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	recv = ledc_channel_config(&ledc_channel1);
	if (recv != ESP_OK) {
		GUA_LOGE("LED timer channel config ERROR!");
	}
}

void set_beep_on(void)
{
	// 设置占空比50%   ((2 ** 13) - 1) * 50% = 4095
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095);
	// Update duty to apply the new value
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void set_beep_tone(uint32 tone)
{ 
	// 设置占空比50%   ((2 ** 13) - 1) * 50% = 4095
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, tone);
	// Update duty to apply the new value
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void set_beep_off(void)
{
	// 设置占空比50%   ((2 ** 13) - 1) * 50% = 4095
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
	// Update duty to apply the new value
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

TaskHandle_t Handle_play_task = NULL;

void play_beep_task(void * pvParameters)
{
	set_beep_tone(1000);
	delay_ms(100);
	set_beep_off();
	delay_ms(10);
	set_beep_tone(6000);
	delay_ms(100);
	set_beep_off();
	vTaskDelete(Handle_play_task);
}

void play_beep_ding(void)
{
	set_beep_tone(1000);
	delay_ms(100);
	set_beep_off();
	// // 创建LED控制任务
	// xTaskCreatePinnedToCore(play_beep_task,            //任务函数
	// 						"keyboard_task",          //任务名称
	// 						2048,                //堆栈大小
	// 						NULL,                //传递参数
	// 						2,                   //任务优先级
	// 						&Handle_play_task,    //任务句柄
	// 						tskNO_AFFINITY);     //无关联，不绑定在任何一个核上
}
