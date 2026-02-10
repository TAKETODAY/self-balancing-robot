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

// motor_airborne_detector.h
#ifndef MOTOR_AIRBORNE_DETECTOR_H
#define MOTOR_AIRBORNE_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// 电机状态
typedef struct {
  float current;      // 电流 (A)
  float speed;        // 转速 (RPM或rad/s)
  float voltage;      // 电压 (V)
  float temperature;  // 温度 (°C)
  float back_emf;     // 反电动势 (V)
  uint32_t timestamp; // 时间戳
} motor_state_t;

// 电机腾空检测
typedef struct {
  float current_threshold;          // 电流阈值 (A)
  float current_variance_threshold; // 电流方差阈值
  float speed_threshold;            // 速度变化阈值
  uint32_t detection_time;          // 检测时间 (ms)

  // 状态
  bool is_airborne;             // 是否腾空
  float current_variance;       // 电流方差
  float baseline_current;       // 基线电流（地面）
  float last_currents[10];      // 电流历史
  int current_index;            // 电流索引
  uint32_t airborne_start_time; // 腾空开始时间

  // 统计
  uint32_t airborne_detections; // 腾空检测次数
  float min_airborne_current;   // 最小腾空电流
} motor_airborne_detector_t;

// API
void motor_airborne_detector_init(motor_airborne_detector_t* detector);
bool motor_airborne_detector_update(motor_airborne_detector_t* detector,
  const motor_state_t* motor_state);
bool motor_airborne_detector_is_airborne(const motor_airborne_detector_t* detector);
void motor_airborne_detector_calibrate(motor_airborne_detector_t* detector,
  float ground_current);

#ifdef __cplusplus
}
#endif


#endif // MOTOR_AIRBORNE_DETECTOR_H
