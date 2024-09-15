/*
 * tool.h
 *
 *  Created on: 2023年8月1日
 *      Author: tianxiaohua
 */

#ifndef SPI_H_
#define SPI_H_

#include "driver_tool.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_log.h"


#define PIN_SPI2_NUM_MISO    GPIO_NUM_13
#define PIN_SPI2_NUM_MOSI    GPIO_NUM_11
#define PIN_SPI2_NUM_CLK     GPIO_NUM_12

#define DMA_CHAN        2
#define PIN_NUM_MISO    13
#define PIN_NUM_MOSI    36
#define PIN_NUM_CLK     37
//#define PIN_NUM_CS      15

//#define SPI_PIN_NRF24_CE
//#define SPI_PIN_NRF24_CSN
//#define SPI_PIN_NRF24_IRQ

extern spi_device_handle_t spi2;
extern spi_device_handle_t spi3;

void spi_write(spi_device_handle_t spi, uint8_t *data, uint32_t len);
esp_err_t spi_read(spi_device_handle_t spi, uint8_t *data, uint8_t data_len);
void spi2_init(void);
void spi3_init(void);

#endif /* APP_USER_TOOL_H_ */
