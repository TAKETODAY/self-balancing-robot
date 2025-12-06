// Copyright 2025 the original author or authors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see [https://www.gnu.org/licenses/]

#include "battery.hpp"
#include "esp/gpio.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <sys/unistd.h>

#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

static const adc_channel_t channel = ADC_CHANNEL_7;

static auto TAG = "battery";

static bool _adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t* out_handle) {

  adc_cali_handle_t handle = NULL;
  esp_err_t ret = ESP_FAIL;
  bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = unit,
      .chan = channel,
      .atten = atten,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
      .unit_id = unit,
      .atten = atten,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

  *out_handle = handle;
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Calibration Success");
  }
  else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
    ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
  }
  else {
    ESP_LOGE(TAG, "Invalid arg or no memory");
  }

  return calibrated;
}

adc_oneshot_unit_handle_t adc1_handle;

bool do_calibration1_chan0 = false;

adc_cali_handle_t adc_cali_handle = NULL;

void showBatteryLED(void* pvParameters) {
  for (;;) {
    const double battery = battery_voltage_read();
    ESP_LOGV("Battery", "%.2fV", battery);

    // 电量显示
    if (battery > 7.0f) {
      digitalWrite(LED_BATTERY_PIN, HIGH);
    }
    else {
      digitalWrite(LED_BATTERY_PIN, LOW);
    }
    sleep(1);
  }
}


void battery_init() {
  const adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  const adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

  do_calibration1_chan0 = _adc_calibration_init(ADC_UNIT_1, channel, ADC_ATTEN_DB_12, &adc_cali_handle);

  pinMode(LED_BATTERY_PIN, OUTPUT);
  digitalWrite(LED_BATTERY_PIN, LOW);

  xTaskCreate(showBatteryLED, "showBatteryLED", 2048, NULL, tskIDLE_PRIORITY, NULL);
}


float battery_voltage_read() {
  int adc_raw;
  int voltage;

  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &adc_raw));
  ESP_LOGV(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, channel, adc_raw);

  if (do_calibration1_chan0) {
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage));
    ESP_LOGV(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, channel, voltage);
  }
  else {
    voltage = adc_raw;
  }
  return (float) voltage * 3.97f / 1000.0f;
}
