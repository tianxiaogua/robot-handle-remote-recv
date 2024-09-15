#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "driver_adc.h"

/*
 * GPIO2、GPIO3、GPIO4、GPIO5、GPIO6作为手柄的输入
 *分别对应的是ADC1_CH1、ADC1_CH2、ADC1_CH3、ADC1_CH4、ADC1_CH5
 * */

#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_11 //配置ADC衰减

static int adc_raw[2][10];
static int voltage[2][10];

adc_oneshot_unit_handle_t adc1_handle;

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

	GUA_LOGI("calibration scheme version is %s", "Curve Fitting");
	adc_cali_curve_fitting_config_t cali_config = {
		.unit_id = unit,
		.atten = atten,
		.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
	if (ret == ESP_OK) {
		calibrated = true;
	}

    *out_handle = handle;
    if (ret == ESP_OK) {
        GUA_LOGI("Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        GUA_LOGI("eFuse not burnt, skip software calibration");
    } else {
        GUA_LOGI("Invalid arg or no memory");
    }

    return calibrated;
}


void driver_init_ADC(void)
{
	//-------------ADC1 Init---------------//
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

	//-------------ADC1 Config---------------//
	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = EXAMPLE_ADC_ATTEN,
	};

	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHAN1, &config));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHAN2, &config));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHAN3, &config));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHAN4, &config));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHAN5, &config));

	//-------------ADC1 Calibration Init---------------//
	adc_cali_handle_t adc1_cali_handle = NULL;
	bool do_calibration1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC_ATTEN, &adc1_cali_handle);

	// test ADC
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_CHAN1, &adc_raw[0][1]));
	GUA_LOGI("ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN1, adc_raw[0][1]);
	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][1], &voltage[0][1]));
		GUA_LOGI("ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN1, voltage[0][1]);
	}
	vTaskDelay(pdMS_TO_TICKS(20));

	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_CHAN2, &adc_raw[0][2]));
	GUA_LOGI("ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN2, adc_raw[0][2]);
	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][2], &voltage[0][2]));
		GUA_LOGI("ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN2, voltage[0][2]);
	}
	vTaskDelay(pdMS_TO_TICKS(20));

	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_CHAN3, &adc_raw[0][3]));
	GUA_LOGI("ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN3, adc_raw[0][3]);
	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][3], &voltage[0][3]));
		GUA_LOGI("ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN3, voltage[0][3]);
	}
	vTaskDelay(pdMS_TO_TICKS(20));

	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_CHAN4, &adc_raw[0][4]));
	GUA_LOGI("ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN4, adc_raw[0][4]);
	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][4], &voltage[0][4]));
		GUA_LOGI("ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN4, voltage[0][4]);
	}
	vTaskDelay(pdMS_TO_TICKS(20));

	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC1_CHAN5, &adc_raw[0][5]));
	GUA_LOGI("ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN5, adc_raw[0][5]);
	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][5], &voltage[0][5]));
		GUA_LOGI("ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN5, voltage[0][5]);
	}
	vTaskDelay(pdMS_TO_TICKS(20));
}


uint32 get_adc_data(uint8 ADC_CHANNEL)
{
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw[0][ADC_CHANNEL]));
	return adc_raw[0][ADC_CHANNEL];
}




