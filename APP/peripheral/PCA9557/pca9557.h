#ifndef __BSP_PCA9557_H__
#define __BSP_PCA9557_H__

/**
 * https://blog.csdn.net/u012121390/article/details/116194561
 * */
#include "esp_err.h"

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */


/*
	0011 A2 A1 A0 R/W
	0011 0  0  0  X		// w: 0x30 r:0x31 (0x18 << 1)
    真实使用i2c地址 ((PCA9557_I2C_SLAVE_ADDR << 1) | R/W)
*/
#define PCA9557_I2C_SLAVE_ADDR  0x18    //(0001 1 A2 A1 A0)

/* 控制寄存器 （CMD）*/
#define PCA9557_CONTROL_REG_0 0x00	// Input Port Register           (R)    BXXXXXXXX (Default)
#define PCA9557_CONTROL_REG_1 0x01	// Output Port Register          (R/W)  B00000000
#define PCA9557_CONTROL_REG_2 0x02	// Polarity Inversion Register   (R/W)  B11110000
#define PCA9557_CONTROL_REG_3 0x03	// Configuration Register        (R/W)  B11111111

typedef enum __pinname
{
	PIN_IO0 = 0, //BIT 0
	PIN_IO1,	 //BIT 1
	PIN_IO2,	 //BIT 2
	PIN_IO3,	 //BIT 3
	PIN_IO4,	 //BIT 4
	PIN_IO5,	 //BIT 5
	PIN_IO6,	 //BIT 6
	PIN_IO7,	 //BIT 7
} snPinName_t;

typedef enum __pinstate {
        IO_LOW  = 0,
        IO_HIGH = 1,
		IO_UNKNOW,
} snPinState_t;

typedef enum __pinmode {
	IO_OUTPUT = 0,
	IO_INPUT  = 1
} snPinMode_t;

typedef enum __pinpolarity{
	IO_NON_INVERTED = 0,
	IO_INVERTED     = 1
} snPinPolarity_t;

/* 外部函数引用 */
esp_err_t bsp_Pca9557Init(void);
esp_err_t bsp_PcaSetIoDirection(snPinName_t pinx, snPinMode_t newMode);
esp_err_t bsp_PcaSetIoStatus(snPinName_t pinx, snPinState_t newState);
esp_err_t bsp_PcaSetInputPolarity(snPinName_t pinx, snPinPolarity_t newPolarity);
snPinState_t bsp_PcaGetIoStatus(snPinName_t pinx);


esp_err_t bsp_Pca9557WriterReg(uint8_t u8I2cSlaveAddr,uint8_t u8Cmd,uint8_t u8Value);
esp_err_t bsp_Pca9557ReadReg(uint8_t u8I2cSlaveAddr,uint8_t u8Cmd,uint8_t *pBuff,uint8_t u8Cnt);
esp_err_t iic_read(uint8_t u8I2cSlaveAddr,uint8_t u8Cmd,uint8_t *pBuff,uint8_t u8Cnt);



esp_err_t init_pca9557(void);
esp_err_t updata_pca9557_io_input(void);
uint8_t read_pca9557_io_input(uint8_t pin);
esp_err_t set_pca9557_io_high(uint8_t pin);
esp_err_t set_pca9557_io_low(uint8_t pin);
#endif

