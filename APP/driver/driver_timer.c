#include "driver_timer.h"

 static bool IRAM_ATTR timer_group_isr_callback(void* args)   //timer_isr
{
	 //中断服务函数，请在这里完成定时器中断要做的事情
	 return true;


}

void bsp_timer_init() //定时器配置
{
	//01 定义定时器配置结构体
	timer_config_t config;
	config.divider = Timer_Divider; //这里配置的是1tick=0.5ms
	config.counter_en = TIMER_PAUSE; //先不使能定时器
	config.counter_dir = TIMER_COUNT_UP;  //向上计数
	config.auto_reload = TIMER_AUTORELOAD_EN; //自动重新装载
	config.alarm_en = TIMER_ALARM_EN; //开启警报，到了阈值之后会触发中断

	//02 控制定时器
	timer_init(BSP_TIMER_GROUP, BSP_TIMER, &config); //配置第0组第0个定时器
	timer_set_counter_value(BSP_TIMER_GROUP, BSP_TIMER, Counter_Value); //计数器初值设置为0
	timer_set_alarm_value(BSP_TIMER_GROUP, BSP_TIMER, Alarm_value); //设置警报值


	//03 使能定时器中断
	timer_enable_intr(BSP_TIMER_GROUP, BSP_TIMER); //使能定时器中断
	timer_isr_callback_add(BSP_TIMER_GROUP, BSP_TIMER, timer_group_isr_callback, 0, 1);//注册中断服务函数
}

void bsp_timer_start() //定时器开始
{
	timer_start(BSP_TIMER_GROUP, BSP_TIMER);
}

void bsp_timer_pause() //定时器停止
{
	timer_pause(BSP_TIMER_GROUP, BSP_TIMER);
}

