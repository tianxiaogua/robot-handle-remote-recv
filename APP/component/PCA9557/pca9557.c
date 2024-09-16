#include "pca9557.h"
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver_iic.h"
#include "driver_gpio.h"
#include "driver_tool.h"

#define PCA9557_I2C_MASTER_SCL_IO           GPIO_NUM_37       /*!< gpio number for I2C master clock */
#define PCA9557_I2C_MASTER_SDA_IO           GPIO_NUM_38      /*!< gpio number for I2C master data  */
#define PCA9557_I2C_MASTER_NUM              I2C_NUM_1    /*!< I2C port number for master dev */
#define PCA9557_I2C_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define PCA9557_I2C_MASTER_TX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */
#define PCA9557_I2C_MASTER_RX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */

#define IIC_ADDRESS 0x30
#define address_read  0x31
#define REGISTER_0_INPUT      0x00 // Register 0 - Input port register
#define REGISTER_1_OUTPUT     0x01 // Register 1 - Output port register
#define REGISTER_2_INVERSION  0x02 // Register 2 - Polarity inversion register
#define REGISTER_3_CONFIG     0x03 // Register 3 - Configuration register

typedef struct pca9557_io {
	uint8_t PIN_MOD_OUT;
	uint8_t PIN_MOD_IN;
} PCA9557_IO_MODE;
PCA9557_IO_MODE io_mode;

/*******************************************************************************
* 名    称： bsp_Pca9557Init
* 功    能： PCA9577 I2C初始化函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2021-05-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
esp_err_t init_pca9557(void)
{
	uint8_t ret = 0;
	uint8_t data_io = 0x8E;  // 10001110 0x8E P1 P2、P3 P7引脚设置为输入，其他设置为输出 1是输入，0是输出 大端在前
	uint8_t data_inversion = 0x00; // 00000000 所有引脚都设置为不反转
	uint8_t data_out = 0xFF;

	io_mode.PIN_MOD_OUT = 0x00;

	GUA_LOGI("init_pca9557");
	ret = dirver_i2c1_init();
	ret = driver_i2c_master_transmit_buf(I2C1, IIC_ADDRESS, REGISTER_3_CONFIG, &data_io, 1); // 设置引脚方向
	GUA_LOGI("Set the pin direction ok");
	ret = driver_i2c_master_transmit_buf(I2C1, IIC_ADDRESS, REGISTER_2_INVERSION, &data_inversion, 1); // 设置引脚反转 不反转
	GUA_LOGI("Set pin inversion to not reverse");
	ret = driver_i2c_master_transmit_buf(I2C1, IIC_ADDRESS, REGISTER_1_OUTPUT, &data_out, 1); // 设置引脚输出电平
	GUA_LOGI("Set the pin output level");
	if(ret != ESP_OK) {
		GUA_LOGI("init_pca9557 Error");
	}
	GUA_LOGI("init_pca9557 OK");
	return ret;
}

/*******************************************************************************
* 名    称： bsp_Pca9557Init
* 功    能： PCA9577 I2C初始化函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2021-05-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
esp_err_t updata_pca9557_io_input(void)
{
	uint8_t recv = driver_i2c_master_receive_buf(I2C1, IIC_ADDRESS, REGISTER_0_INPUT, &io_mode.PIN_MOD_OUT, 1);
	if(recv == ESP_OK) {
//		ESP_LOGE(TAG_PCA9557, "%2x", io_mode.PIN_MOD_OUT);
		return ESP_OK;
	}
	else {
		GUA_LOGE("read_pca9557_io_input ERROR");
		return -1;
	}


}

/*******************************************************************************
* 名    称： bsp_Pca9557Init
* 功    能： PCA9577 I2C初始化函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2021-05-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t read_pca9557_io_input(uint8_t pin)
{
	if(io_mode.PIN_MOD_OUT & (1 << pin)){
		return 1;
	}
	else {
		return 0;
	}
}

/*******************************************************************************
* 名    称： bsp_Pca9557Init
* 功    能： PCA9577 I2C初始化函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2021-05-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
esp_err_t set_pca9557_io_high(uint8_t pin)
{
	uint8_t ret = 0;
	io_mode.PIN_MOD_IN |= (1 << pin);
	ret = driver_i2c_master_transmit_buf(I2C1, IIC_ADDRESS, REGISTER_1_OUTPUT, &io_mode.PIN_MOD_IN, 1); // 设置引脚输出电平
	if(ret != ESP_OK) {
		GUA_LOGI("set_pca9557_io_high OK");
	}
	return ret;
}

/*******************************************************************************
* 名    称： bsp_Pca9557Init
* 功    能： PCA9577 I2C初始化函数
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2021-05-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
esp_err_t set_pca9557_io_low(uint8_t pin)
{
	uint8_t ret = 0;
	io_mode.PIN_MOD_IN &= ~(1 << pin);
	ret = driver_i2c_master_transmit_buf(I2C1, IIC_ADDRESS, REGISTER_1_OUTPUT, &io_mode.PIN_MOD_IN, 1); // 设置引脚输出电平
	if(ret != ESP_OK) {
		GUA_LOGI("set_pca9557_io_high OK");
	}
	return ret;
}
