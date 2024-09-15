/*
 * tool.c
 *
 *  Created on: 2023年8月1日
 *      Author: tianxiaohua
 */

#include "driver_spi.h"

/*
.3 GPIO矩阵和IO_MUX
ESP32的大多数外设信号都直接连接到其专用的IO_MUX引脚。但是，也可以使用GPIO矩阵将信号转换到任何其他可用的引脚。如果至少一个信号通过GPIO矩阵转换，则所有信号都将通过GPIO矩阵转换。

GPIO矩阵引入了转换灵活性，但也带来了以下缺点：

增加了MISO信号的输入延迟，这更可能违反MISO设置时间。如果SPI需要高速运行，请使用专用的IO_MUX引脚。
如果使用IO_MUX引脚，则允许信号的时钟频率最多为40 MHz，而时钟频率最高为80 MHz。
SPI总线的IO_MUX引脚如下所示

引脚对应的
GPIO	SPI2	SPI3
CS0 *	15	5
SCLK	14	18
MISO	12	19
MOSI	13	23
QUADWP	2	22
QUADHD	4	21
仅连接到总线的第一个设备可以使用CS0引脚。
            
原文链接：https://blog.csdn.net/qq_36347513/article/details/117299126
*/


spi_device_handle_t spi2;
void spi2_init(void)
{
	esp_err_t ret;
	GUA_LOGI("Initializing bus SPI%d...", SPI2_HOST+1);

	static spi_bus_config_t buscfg={
		.miso_io_num = PIN_SPI2_NUM_MISO,                // MISO信号线
		.mosi_io_num = PIN_SPI2_NUM_MOSI,                // MOSI信号线
		.sclk_io_num = PIN_SPI2_NUM_CLK,                 // SCLK信号线
		.quadwp_io_num = -1,                        // WP信号线，专用于QSPI的D2
		.quadhd_io_num = -1,                        // HD信号线，专用于QSPI的D3
		.max_transfer_sz = 64*8,                    // 最大传输数据大小
	};

	static spi_device_interface_config_t devcfg={
		.clock_speed_hz = SPI_MASTER_FREQ_10M,      // Clock out at 10 MHz,
		.mode = 0,                                  // SPI mode 0
		.spics_io_num = -1,                         // < CS GPIO pin for this device, or -1 if not used
		.queue_size = 7,                            // 传输队列大小，决定了等待传输数据的数量
	};

	//Initialize the SPI bus
	ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK) {
		GUA_LOGE("Initialize the SPI bus ERROR!");
	}
	ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi2);
	if (ret != ESP_OK) {
		GUA_LOGE("SPI add device bus ERROR!");
	}
}


spi_device_handle_t spi3;
void spi3_init(void)
{
	esp_err_t ret;
	GUA_LOGI("Initializing bus SPI%d...", SPI3_HOST+1);

	spi_bus_config_t buscfg={
		.miso_io_num = PIN_NUM_MISO,                // MISO信号线
		.mosi_io_num = PIN_NUM_MOSI,                // MOSI信号线
		.sclk_io_num = PIN_NUM_CLK,                 // SCLK信号线
		.quadwp_io_num = -1,                        // WP信号线，专用于QSPI的D2
		.quadhd_io_num = -1,                        // HD信号线，专用于QSPI的D3
		.max_transfer_sz = 64*8,                    // 最大传输数据大小
	};

	spi_device_interface_config_t devcfg={
		.clock_speed_hz = SPI_MASTER_FREQ_80M,      // Clock out at 10 MHz,
		.mode = 0,                                  // SPI mode 0
		.spics_io_num = -1,
		.queue_size = 7,                            // 传输队列大小，决定了等待传输数据的数量
	};

	//Initialize the SPI bus
	ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK) {
		GUA_LOGE("Initialize the SPI bus ERROR!");
	}
	ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi3);
	if (ret != ESP_OK) {
		GUA_LOGE("SPI add device bus ERROR!");
	}
}


void spi_write(spi_device_handle_t spi, uint8_t *data, uint32_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction

    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    if(ret != ESP_OK) {
    	GUA_LOGE("SPI2 write error!\n");
	}
}

esp_err_t spi_read(spi_device_handle_t spi, uint8_t *data, uint8 data_len)
{
    spi_transaction_t t;

    memset(&t, 0, sizeof(t));
    t.length=8*data_len;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;
    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    if(ret != ESP_OK) {
    	GUA_LOGE("SPI read error!\n");
    }
    *data = t.rx_data[0];

    return ret;
}

esp_err_t spi_sendAndRecv(spi_device_handle_t spi, uint8_t *txdata, uint8 txdata_len, uint8_t *rxdata, uint8 rxdata_len)
{
    esp_err_t ret;
    spi_transaction_t t;
	uint8_t recv[4];

    memset(&t, 0, sizeof(t));
    memset(recv, 0xAA, 4);
    t.length=8;
    t.tx_buffer=&txdata;
    t.rx_buffer=recv;
    ret = spi_device_polling_transmit(spi, &t);
    if(ret != ESP_OK) {
		GUA_LOGE("SPI sand and recv error!\n");
		return ESP_FAIL;
	}
    return ESP_OK;
}

