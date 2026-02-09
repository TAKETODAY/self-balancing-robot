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

// battery.c

#include "battery.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

// 日志标签
static const char* TAG = "BATTERY";

// 静态变量
static battery_info_t s_battery_info = { 0 };

static battery_config_t s_config = {
  .alpha = 0.1f,
  .update_interval = 1000, // 1秒更新一次
  .adc_samples = 16,
  .adc_channel = ADC1_CHANNEL_0
};

static esp_adc_cal_characteristics_t* s_adc_chars = NULL;
static battery_callback_t s_callback = NULL;
static battery_low_callback_t s_low_callback = NULL;
static battery_critical_callback_t s_critical_callback = NULL;

// 不同电池类型的电压曲线（单位：V）
typedef struct {
  float full; // 满电电压
  float empty; // 空电电压
  float low; // 低电量警告
  float critical; // 临界电量
  const char* name; // 电池名称
} battery_profile_t;

// 常见电池配置
static const battery_profile_t s_battery_profiles[] = {
  // 1S锂电池 (3.7V)
  [BATTERY_TYPE_LIPO_1S] = {
    .full = 4.20f,
    .empty = 3.00f,
    .low = 3.30f,
    .critical = 3.10f,
    .name = "1S LiPo"
  },

  // 2S锂电池 (7.4V)
  [BATTERY_TYPE_LIPO_2S] = {
    .full = 8.40f,
    .empty = 6.00f,
    .low = 6.60f,
    .critical = 6.20f,
    .name = "2S LiPo"
  },

  // 3S锂电池 (11.1V)
  [BATTERY_TYPE_LIPO_3S] = {
    .full = 12.60f,
    .empty = 9.00f,
    .low = 9.90f,
    .critical = 9.30f,
    .name = "3S LiPo"
  },

  // 4S锂电池 (14.8V)
  [BATTERY_TYPE_LIPO_4S] = {
    .full = 16.80f,
    .empty = 12.00f,
    .low = 13.20f,
    .critical = 12.40f,
    .name = "4S LiPo"
  },

  // 12V铅酸电池
  [BATTERY_TYPE_LEAD_ACID_12V] = {
    .full = 13.80f,
    .empty = 10.50f,
    .low = 11.50f,
    .critical = 10.80f,
    .name = "12V Lead Acid"
  },

  // 镍氢电池 (假设7.2V)
  [BATTERY_TYPE_NIMH] = {
    .full = 8.40f,
    .empty = 6.00f,
    .low = 6.80f,
    .critical = 6.40f,
    .name = "NiMH"
  }
};

// 初始化ADC
static bool init_adc(void) {
  // 配置ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(s_config.adc_channel, ADC_ATTEN_DB_11);

  // 校准特性
  s_adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  if (!s_adc_chars) {
    ESP_LOGE(TAG, "Failed to allocate ADC calibration memory");
    return false;
  }

  // 获取校准值（使用默认的Vref）
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
    ADC_UNIT_1,
    ADC_ATTEN_DB_11,
    ADC_WIDTH_BIT_12,
    1100, // 默认Vref
    s_adc_chars
  );

  if (val_type == ESP_ADC_CAL_VAL_NOT_SUPPORTED) {
    ESP_LOGW(TAG, "ADC calibration not supported, using default");
  }
  else if (val_type == ESP_ADC_CAL_VAL_DEFAULT_VREF) {
    ESP_LOGI(TAG, "ADC calibration using default Vref");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    ESP_LOGI(TAG, "ADC calibration using eFuse Vref");
  }

  return true;
}

// 读取ADC电压（多次采样平均）
static float read_adc_voltage(void) {
  if (!s_adc_chars) {
    ESP_LOGE(TAG, "ADC not initialized");
    return 0.0f;
  }

  uint32_t adc_sum = 0;

  // 多次采样取平均
  for (int i = 0; i < s_config.adc_samples; i++) {
    adc_sum += adc1_get_raw(s_config.adc_channel);
    vTaskDelay(1 / portTICK_PERIOD_MS); // 短暂延时
  }

  s_battery_info.adc_raw = adc_sum / s_config.adc_samples;

  // 转换为电压（mV）
  uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(
    s_battery_info.adc_raw,
    s_adc_chars
  );

  return voltage_mv / 1000.0f; // 转换为V
}

// 计算分压后的实际电池电压
// 假设使用电阻分压电路，分压比为 R2/(R1+R2)
// 实际电压 = ADC电压 * (R1+R2)/R2
static float calculate_battery_voltage(float adc_voltage) {
  // 这里假设使用100k和33k电阻分压，分压比 = 33/(100+33) ≈ 0.248
  // 实际使用时需要根据你的电路调整
  const float R1 = 100000.0f; // 100kΩ
  const float R2 = 33000.0f; // 33kΩ
  const float voltage_divider_ratio = R2 / (R1 + R2);

  return adc_voltage / voltage_divider_ratio;
}

// 一阶低通滤波器
static float low_pass_filter(float new_value, float old_value, float alpha) {
  return old_value * (1.0f - alpha) + new_value * alpha;
}

// 根据电压计算电量百分比
float battery_calculate_percentage(float voltage, battery_type_t type) {
  if (type >= sizeof(s_battery_profiles) / sizeof(s_battery_profiles[0])) {
    type = BATTERY_TYPE_LIPO_2S; // 默认使用2S锂电池
  }

  const battery_profile_t* profile = &s_battery_profiles[type];

  // 边界检查
  if (voltage >= profile->full) {
    return 100.0f;
  }

  if (voltage <= profile->empty) {
    return 0.0f;
  }

  // 线性插值计算百分比
  float percentage = 100.0f * (voltage - profile->empty) / (profile->full - profile->empty);

  // 锂电池放电曲线不是线性的，这里可以添加非线性校正
  // 简单的非线性校正：在低电压区域使用不同的斜率
  if (type <= BATTERY_TYPE_LIPO_4S) {
    // 锂电池
    // 锂电池放电曲线：高电压段下降快，中间平缓，低电压段下降快
    if (voltage > profile->low) {
      // 高电压段：稍微调整曲线
      percentage = percentage * 0.95f + 5.0f;
    }
    else if (voltage < profile->low) {
      // 低电压段：快速下降
      float low_range_percentage = 100.0f * (voltage - profile->empty) / (profile->low - profile->empty);
      percentage = low_range_percentage * 0.7f;
    }
  }

  // 限制在0-100范围内
  if (percentage > 100.0f) percentage = 100.0f;
  if (percentage < 0.0f) percentage = 0.0f;

  return percentage;
}

// 确定电池状态
static battery_state_t determine_battery_state(float voltage, float percentage) {
  battery_type_t type = s_battery_info.type;
  const battery_profile_t* profile = &s_battery_profiles[type];

  if (voltage <= profile->critical) {
    return BATTERY_STATE_CRITICAL;
  }

  if (voltage <= profile->low) {
    return BATTERY_STATE_LOW;
  }

  if (percentage >= 98.0f && voltage >= profile->full - 0.1f) {
    return BATTERY_STATE_FULL;
  }

  // 这里可以添加充电检测逻辑
  // 如果有充电检测引脚，可以在这里读取

  return BATTERY_STATE_NORMAL;
}

// 初始化电池监控
bool battery_init(battery_type_t type, float capacity_mah,
  uint32_t adc_channel, const battery_config_t* config) {
  // 参数验证
  if (type >= sizeof(s_battery_profiles) / sizeof(s_battery_profiles[0])) {
    ESP_LOGE(TAG, "Invalid battery type: %d", type);
    return false;
  }

  if (capacity_mah <= 0) {
    ESP_LOGE(TAG, "Invalid battery capacity: %.2f mAh", capacity_mah);
    return false;
  }

  // 保存配置
  s_battery_info.type = type;
  s_battery_info.capacity_mah = capacity_mah;

  if (config) {
    memcpy(&s_config, config, sizeof(battery_config_t));
  }

  if (adc_channel != (uint32_t) -1) {
    s_config.adc_channel = adc_channel;
  }

  // 获取电池配置
  const battery_profile_t* profile = &s_battery_profiles[type];

  // 初始化电池信息
  s_battery_info.voltage_full = profile->full;
  s_battery_info.voltage_empty = profile->empty;
  s_battery_info.voltage_low = profile->low;
  s_battery_info.voltage_critical = profile->critical;

  // 初始化ADC
  if (!init_adc()) {
    return false;
  }

  // 首次读取电压
  battery_update();

  ESP_LOGI(TAG, "Battery monitor initialized: %s %.2f mAh",
    profile->name, capacity_mah);
  ESP_LOGI(TAG, "Voltage: %.2fV, Percentage: %.1f%%",
    s_battery_info.voltage, s_battery_info.percentage);

  return true;
}

// 更新电池信息
bool battery_update(void) {
  static uint32_t last_update_time = 0;
  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

  // 检查更新时间间隔
  if (current_time - last_update_time < s_config.update_interval) {
    return false;
  }

  last_update_time = current_time;

  // 读取ADC电压
  float adc_voltage = read_adc_voltage();

  // 计算实际电池电压
  float raw_voltage = calculate_battery_voltage(adc_voltage);

  // 低通滤波
  s_battery_info.voltage_filtered = low_pass_filter(
    raw_voltage,
    s_battery_info.voltage_filtered,
    s_config.alpha
  );

  // 使用滤波后的电压
  s_battery_info.voltage = s_battery_info.voltage_filtered;

  // 计算电量百分比
  s_battery_info.percentage = battery_calculate_percentage(
    s_battery_info.voltage,
    s_battery_info.type
  );

  // 确定电池状态
  battery_state_t old_state = s_battery_info.state;
  s_battery_info.state = determine_battery_state(
    s_battery_info.voltage,
    s_battery_info.percentage
  );

  // 更新时间戳
  s_battery_info.last_update = current_time;

  // 状态变化回调
  if (s_battery_info.state != old_state) {
    // 低电量警告
    if (s_battery_info.state == BATTERY_STATE_LOW && s_low_callback) {
      s_low_callback();
    }

    // 临界电量警告
    if (s_battery_info.state == BATTERY_STATE_CRITICAL && s_critical_callback) {
      s_critical_callback();
    }
  }

  // 常规回调
  if (s_callback) {
    s_callback(&s_battery_info);
  }

  return true;
}

// 获取电池信息
const battery_info_t* battery_get_info(void) {
  return &s_battery_info;
}

// 设置回调
void battery_set_callback(battery_callback_t callback) {
  s_callback = callback;
}

void battery_set_low_callback(battery_low_callback_t callback) {
  s_low_callback = callback;
}

void battery_set_critical_callback(battery_critical_callback_t callback) {
  s_critical_callback = callback;
}

// 估算剩余运行时间（小时）
float battery_estimate_runtime(float current_draw) {
  if (current_draw <= 0 || s_battery_info.capacity_mah <= 0) {
    return 0.0f;
  }

  // 剩余容量 = 总容量 * 当前百分比
  float remaining_mah = s_battery_info.capacity_mah * s_battery_info.percentage / 100.0f;

  // 运行时间 = 剩余容量 / 电流消耗
  return remaining_mah / current_draw;
}

// 重置库仑计数
void battery_reset_coulomb_count(void) {
  s_battery_info.consumed_mah = 0.0f;
}
