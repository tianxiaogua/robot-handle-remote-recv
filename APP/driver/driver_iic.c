/*
 * driver_iic.c
 *
 *  Created on: 2024年7月13日
 *      Author: tianxiaohua
 */
#include "driver_iic.h"
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver_gpio.h"

#define I2C0_MASTER_SCL_IO           GPIO_NUM_9       /*!< gpio number for I2C master clock */
#define I2C0_MASTER_SDA_IO           GPIO_NUM_10      /*!< gpio number for I2C master data  */
#define I2C0_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define I2C0_MASTER_TX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */
#define I2C0_MASTER_RX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */

#define I2C1_MASTER_SCL_IO           GPIO_NUM_37       /*!< gpio number for I2C master clock */
#define I2C1_MASTER_SDA_IO           GPIO_NUM_38      /*!< gpio number for I2C master data  */
#define I2C1_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */
#define I2C1_MASTER_TX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */
#define I2C1_MASTER_RX_BUF_DISABLE   0       /*!< I2C master doesn't need buffer */

esp_err_t dirver_i2c0_init(void)
{
	int32 recv = 0;
	int32 i2c_master_port = I2C_NUM_0;
	static i2c_config_t conf_i2c0;
	conf_i2c0.mode = I2C_MODE_MASTER;
	conf_i2c0.sda_io_num = I2C0_MASTER_SDA_IO;
	conf_i2c0.sda_pullup_en = GPIO_PULLUP_DISABLE;
	conf_i2c0.scl_io_num = I2C0_MASTER_SCL_IO;
	conf_i2c0.scl_pullup_en = GPIO_PULLUP_DISABLE;
	conf_i2c0.master.clk_speed = I2C0_MASTER_FREQ_HZ;

	recv = i2c_param_config(i2c_master_port, &conf_i2c0);
	if(recv != ESP_OK){
		GUA_LOGE("ESP32 i2c0 param config error!");
		return REV_ERR;
	}
	recv =  i2c_driver_install(i2c_master_port, conf_i2c0.mode, I2C0_MASTER_RX_BUF_DISABLE, I2C0_MASTER_TX_BUF_DISABLE, 0);
	if(recv != ESP_OK){
		GUA_LOGE("ESP32 iic_0 init error!");
		return REV_ERR;
	}
	return REV_OK;
}

esp_err_t dirver_i2c1_init(void)
{
	int recv = 0;
	int i2c_master_port = I2C_NUM_1;
	static i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C1_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	conf.scl_io_num = I2C1_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	conf.master.clk_speed = I2C1_MASTER_FREQ_HZ;
	i2c_param_config(i2c_master_port, &conf);
	recv =  i2c_driver_install(i2c_master_port, conf.mode, I2C1_MASTER_RX_BUF_DISABLE, I2C1_MASTER_TX_BUF_DISABLE, 0);
	if(recv != ESP_OK){
		GUA_LOGE("ESP32 iic_1 init error!");
		return recv;
	}
	return ESP_OK;
}


esp_err_t driver_i2c_master_transmit_buf(uint8_t i2c_master_port, uint8_t iic_address, uint8_t reg_address, uint8_t *data_buf, uint32_t data_len)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, iic_address, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
	i2c_master_write(cmd, data_buf, data_len, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_master_port, cmd, 10);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK) {
		GUA_LOGE("IIC write error!");
	}
	return ret;
}


esp_err_t driver_i2c_master_receive_buf(uint8_t i2c_master_port, uint8_t iic_address, uint8_t reg_address, uint8_t *data_buf, uint32_t data_len)
{
	esp_err_t ret ;
	i2c_cmd_handle_t wr_cmd = i2c_cmd_link_create();
	i2c_master_start(wr_cmd);
	i2c_master_write_byte(wr_cmd, iic_address, ACK_CHECK_EN);
	i2c_master_write_byte(wr_cmd, reg_address, ACK_CHECK_EN);
	i2c_master_stop(wr_cmd);
	ret = i2c_master_cmd_begin(i2c_master_port, wr_cmd, 100);
	i2c_cmd_link_delete(wr_cmd);
	if(ret != ESP_OK) {
		GUA_LOGE("IIC read cmd write error!");
	}

	i2c_cmd_handle_t rd_cmd = i2c_cmd_link_create();
	i2c_master_start(rd_cmd);
	i2c_master_write_byte(rd_cmd, iic_address+0x01, ACK_CHECK_EN);
	i2c_master_read(rd_cmd, data_buf, data_len, I2C_MASTER_LAST_NACK);
	i2c_master_stop(rd_cmd);
	ret = i2c_master_cmd_begin(i2c_master_port, rd_cmd, 100);
	i2c_cmd_link_delete(rd_cmd);
	if(ret != ESP_OK) {
		GUA_LOGE("IIC write error!");
	}
	return ret;
}


esp_err_t esp32_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write(cmd, data, length, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
    	GUA_LOGE("esp32 write iic error!");
    }
    return ret;
}

esp_err_t esp32_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
    if (length == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
		GUA_LOGE("esp32 write iic error!");
	}
    return ret;
}



