/*
 * led.c
 *
 *  Created on: 2023年2月18日
 *      Author: 29602
 */

#include "driver_pwm.h"
#include "driver_tool.h"


void ledc_init(void)
{
	int32 ret = 0;
	// 设置LEDC周边配置
	// 准备并应用LEDC PWM定时器配置
	ledc_timer_config_t ledc_timer1 = {
		.speed_mode       = LEDC_LOW_SPEED_MODE,
		.timer_num        = LEDC_TIMER_0,
		.duty_resolution  = LEDC_TIMER_13_BIT, // 设置分辨率为13位
		.freq_hz          = 5000,  // 频率单位为赫兹。频率设定为5千赫
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ret = ledc_timer_config(&ledc_timer1);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_timer1 config ERROR!");
	}

	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel1 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_0,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE, // 禁用ＬＥＤＣ中断
		.gpio_num       = 17,  // 绑定GPIO
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	ret = ledc_channel_config(&ledc_channel1);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_channel1 config ERROR!");
	}

	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel2 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_1,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = 16,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	ret = ledc_channel_config(&ledc_channel2);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_channel2 config ERROR!");
	}

	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel3 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_2,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = 3,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	ret = ledc_channel_config(&ledc_channel3);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_channel3 config ERROR!");
	}

	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel4 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_3,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = 2,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	ret = ledc_channel_config(&ledc_channel4);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_channel4 config ERROR!");
	}
	// 准备并应用LEDC PWM通道配置
	ledc_channel_config_t ledc_channel5 = {
		.speed_mode     = LEDC_LOW_SPEED_MODE,
		.channel        = LEDC_CHANNEL_4,
		.timer_sel      = LEDC_TIMER_0,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = 1,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0
	};
	ret = ledc_channel_config(&ledc_channel5);
	if (ret != ESP_OK) {
		GUA_LOGE("led timer ledc_channel5 config ERROR!");
	}
}

void ledc_set(ledc_channel_t channel, uint32_t duty)
{
	 // 设置占空比50%   ((2 ** 13) - 1) * 50% = 4095
	ledc_set_duty(LEDC_LOW_SPEED_MODE, channel,duty);
	// Update duty to apply the new value
	ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}
