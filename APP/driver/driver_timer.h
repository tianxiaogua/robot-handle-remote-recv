#ifndef driver_timer_H_
#define driver_timer_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"


//定义定时器组
#define BSP_TIMER_GROUP TIMER_GROUP_0
#define BSP_TIMER TIMER_0


//定义参数
#define Timer_Divider 40000 //配置时钟，因为时钟是80M，40000以后，增加一个数字0.5ms
#define Counter_Value 0 //定时器初值
#define Alarm_value   1000 //警报阈值，这里10000*0.5ms = 5s

//函数声明
void bsp_timer_init();//初始化定时器
void bsp_timer_start(); //打开定时器
void bsp_timer_pause(); //关闭定时器

#endif /* APP_USER_TOOL_H_ */
