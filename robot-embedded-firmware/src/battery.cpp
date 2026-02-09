// Copyright 2025 - 2026 the original author or authors.
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
#include "logging.hpp"
#include "esp/gpio.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <sys/unistd.h>

#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

#define LED_BATTERY_PIN gpio_num_t::GPIO_NUM_13
#define BAT_PIN gpio_num_t::GPIO_NUM_35

static auto TAG = "battery";
static constexpr adc_channel_t channel = ADC_CHANNEL_7;

adc_oneshot_unit_handle_t adc1_handle;

bool do_calibration1_chan0 = false;

adc_cali_handle_t adc_cali_handle = nullptr;

static struct {
  float full;
  float empty;
  float low;
  float critical;
} profile = {
  .full = 8.40f,
  .empty = 6.00f,
  .low = 6.60f,
  .critical = 6.20f
};

static bool _adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t* out_handle) {
  adc_cali_handle_t handle = nullptr;
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

// 根据电压计算电量百分比
float battery_calculate_percentage(const float voltage) {
  // 边界检查
  if (voltage >= profile.full) {
    return 100.0f;
  }

  if (voltage <= profile.empty) {
    return 0.0f;
  }

  // 线性插值计算百分比
  float percentage = 100.0f * (voltage - profile.empty) / (profile.full - profile.empty);

  // 锂电池
  // 锂电池放电曲线：高电压段下降快，中间平缓，低电压段下降快
  if (voltage > profile.low) {
    // 高电压段：稍微调整曲线
    percentage = percentage * 0.95f + 5.0f;
  }
  else if (voltage < profile.low) {
    // 低电压段：快速下降
    const float low_range_percentage = 100.0f * (voltage - profile.empty) / (profile.low - profile.empty);
    percentage = low_range_percentage * 0.7f;
  }

  // 限制在0-100范围内
  if (percentage > 100.0f) {
    percentage = 100.0f;
  }
  if (percentage < 0.0f) {
    percentage = 0.0f;
  }
  return percentage;
}


void showBatteryLED(void* pvParameters) {
  for (;;) {
    const float voltage = battery_voltage_read();
    log_info("%.2fV", voltage);

    const float percentage = battery_calculate_percentage(voltage);
    log_info("%.2f", percentage);

    // 电量显示
    if (voltage > 7.0f) {
      digitalWrite(LED_BATTERY_PIN, HIGH);
    }
    else {
      digitalWrite(LED_BATTERY_PIN, LOW);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    static int count = 0;
    if (++count % 10 == 0) {
      UBaseType_t stack_remain = uxTaskGetStackHighWaterMark(nullptr);
      ESP_LOGW(TAG, "栈空间剩余: %d 字节", stack_remain);
    }
  }
}


void battery_init() {
  constexpr adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  constexpr adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

  do_calibration1_chan0 = _adc_calibration_init(ADC_UNIT_1, channel, ADC_ATTEN_DB_12, &adc_cali_handle);

  pinMode(LED_BATTERY_PIN, OUTPUT);
  digitalWrite(LED_BATTERY_PIN, LOW);

  xTaskCreate(showBatteryLED, "bat-loop", 3000, nullptr, tskIDLE_PRIORITY, nullptr);
}

float battery_capacity_read() {
  const float voltage = battery_voltage_read();
  return battery_calculate_percentage(voltage);
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
  return static_cast<float>(voltage) * 3.97f / 1000.0f;
}
