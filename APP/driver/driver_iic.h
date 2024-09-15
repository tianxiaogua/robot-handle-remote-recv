/*
 * driver_iic.h
 *
 *  Created on: 2024年7月13日
 *      Author: tianxiaohua
 */

#ifndef DRIVER_IIC_H_
#define DRIVER_IIC_H_

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver_gpio.h"

#define ACK_CHECK_EN  0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                        /*!< I2C master will not check ack from slave */

#define I2C0 I2C_NUM_0
#define I2C1 I2C_NUM_1

esp_err_t dirver_i2c1_init(void);

esp_err_t dirver_i2c0_init(void);

esp_err_t driver_i2c_master_transmit_buf(uint8_t i2c_master_port, uint8_t iic_address, uint8_t reg_address, uint8_t *data_buf, uint32_t data_len);
esp_err_t driver_i2c_master_receive_buf(uint8_t i2c_master_port, uint8_t iic_address, uint8_t reg_address, uint8_t *data_buf, uint32_t data_len);

esp_err_t esp32_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);
esp_err_t esp32_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data);

#endif /* APP_DRIVER_DRIVER_IIC_H_ */
