/*
 * tool.c
 *
 *  Created on: 2023年8月1日
 *      Author: tianxiaohua
 */

#include "driver_tool.h"
#include "kalman.h"
#include "stdio.h"


/******************************************************
 * @作者：田小呱
 * @时间：2023.8
 * @功能：延时函数 单位1ms
 * @输入：延时时间 1ms
 * @输出：无
 ******************************************************/
void delay_ms(uint32 time)
{
	vTaskDelay(time / portTICK_PERIOD_MS); // 输入100 延时1秒钟
}



void driver_kalman_Init(void)
{
	Kalman_Init();
	Kalman1_Init();
	Kalman2_Init();
	Kalman3_Init();
}

float driver_kalman_filter(uint8 channel, float input)
{
	switch (channel) {
		case filter_channer1: {
			return kalman_filter(&kfp, input);
		} break;
		case filter_channer2: {
			return kalman_filter(&kfp1, input);
		} break;
		case filter_channer3: {
			return kalman_filter(&kfp2, input);
		} break;
		case filter_channer4: {
			return kalman_filter(&kfp3, input);
		} break;
		default :
			return 0;
	}
}
