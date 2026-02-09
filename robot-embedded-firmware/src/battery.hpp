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

#pragma once

#define LED_BATTERY_PIN gpio_num_t::GPIO_NUM_13
#define BAT_PIN gpio_num_t::GPIO_NUM_35

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// 电池类型定义
typedef enum {
  BATTERY_TYPE_LIPO_1S = 0, // 1S锂电池 (3.7V)
  BATTERY_TYPE_LIPO_2S, // 2S锂电池 (7.4V)
  BATTERY_TYPE_LIPO_3S, // 3S锂电池 (11.1V)
  BATTERY_TYPE_LIPO_4S, // 4S锂电池 (14.8V)
  BATTERY_TYPE_LEAD_ACID_12V, // 12V铅酸电池
  BATTERY_TYPE_NIMH, // 镍氢电池
} battery_type_t;

// 电池状态
typedef enum {
  BATTERY_STATE_NORMAL = 0,
  BATTERY_STATE_CHARGING,
  BATTERY_STATE_FULL,
  BATTERY_STATE_LOW,
  BATTERY_STATE_CRITICAL,
  BATTERY_STATE_FAULT,
} battery_state_t;

// 电池信息结构
typedef struct {
  float voltage; // 当前电压 (V)
  float percentage; // 电量百分比 (0-100)
  battery_state_t state; // 电池状态
  battery_type_t type; // 电池类型
  uint32_t last_update; // 最后更新时间戳
  uint16_t adc_raw; // ADC原始值
  float voltage_filtered; // 滤波后的电压
  float current; // 电流 (A) - 如果有电流传感器
  float capacity_mah; // 电池容量 (mAh)
  float consumed_mah; // 已消耗电量 (mAh)

  // 电压阈值
  float voltage_full; // 满电电压
  float voltage_empty; // 空电电压
  float voltage_low; // 低电量警告电压
  float voltage_critical; // 临界电量电压
} battery_info_t;

// 滤波配置
typedef struct {
  float alpha; // 滤波系数 (0.0-1.0)，越小滤波越强
  uint32_t update_interval; // 更新间隔 (ms)
  uint8_t adc_samples; // ADC采样次数
  uint32_t adc_channel; // ADC通道 (例如ADC1_CHANNEL_0)
} battery_config_t;

// 回调函数类型
typedef void (*battery_callback_t)(const battery_info_t* info);
typedef void (*battery_low_callback_t)(void);
typedef void (*battery_critical_callback_t)(void);

// 获取电池信息
const battery_info_t* battery_get_info(void);
bool battery_update(void);

// 设置回调
void battery_set_callback(battery_callback_t callback);
void battery_set_low_callback(battery_low_callback_t callback);
void battery_set_critical_callback(battery_critical_callback_t callback);

// 工具函数
float battery_calculate_percentage(float voltage, battery_type_t type);
float battery_estimate_runtime(float current_draw); // 估算剩余运行时间
void battery_reset_coulomb_count(void); // 重置库仑计数


void battery_init();

float battery_voltage_read();

typedef void (*battery_callback_t)(const battery_info_t* info);
typedef void (*battery_low_callback_t)(void);
typedef void (*battery_critical_callback_t)(void);

#ifdef __cplusplus
}
#endif
