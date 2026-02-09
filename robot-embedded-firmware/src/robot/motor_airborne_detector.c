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

// motor_airborne_detector.c

#include "robot/motor_airborne_detector.h"
#include <string.h>
#include <math.h>

// 初始化
void motor_airborne_detector_init(motor_airborne_detector_t* detector) {
  if (!detector) return;

  memset(detector, 0, sizeof(motor_airborne_detector_t));

  detector->current_threshold = 0.3f; // 0.3A阈值
  detector->current_variance_threshold = 0.05f; // 电流方差阈值
  detector->speed_threshold = 10.0f; // 速度变化阈值
  detector->detection_time = 50; // 50ms

  detector->baseline_current = 1.0f; // 默认基线电流
  detector->min_airborne_current = 999.0f; // 初始化为大值
}

// 更新检测
bool motor_airborne_detector_update(motor_airborne_detector_t* detector,
  const motor_state_t* motor_state) {
  if (!detector || !motor_state) return false;

  // 保存电流历史
  detector->last_currents[detector->current_index] = fabsf(motor_state->current);
  detector->current_index = (detector->current_index + 1) % 10;

  // 计算电流方差（最近10个样本）
  float sum = 0.0f;
  float sum_sq = 0.0f;
  int count = 0;

  for (int i = 0; i < 10; i++) {
    if (detector->last_currents[i] > 0) {
      sum += detector->last_currents[i];
      sum_sq += detector->last_currents[i] * detector->last_currents[i];
      count++;
    }
  }

  if (count > 1) {
    float mean = sum / count;
    detector->current_variance = (sum_sq / count) - (mean * mean);
  }

  // 腾空检测逻辑
  bool airborne_detected = false;

  // 条件1: 电流显著低于基线
  if (fabsf(motor_state->current) < detector->baseline_current * 0.3f) {
    airborne_detected = true;
  }

  // 条件2: 电流方差小（没有负载变化）
  if (detector->current_variance < detector->current_variance_threshold) {
    airborne_detected = true;
  }

  // 更新状态
  if (airborne_detected) {
    if (!detector->is_airborne) {
      detector->is_airborne = true;
      detector->airborne_start_time = motor_state->timestamp;
      detector->airborne_detections++;
    }

    // 更新最小电流记录
    if (fabsf(motor_state->current) < detector->min_airborne_current) {
      detector->min_airborne_current = fabsf(motor_state->current);
    }
  }
  else {
    detector->is_airborne = false;
  }

  return detector->is_airborne;
}

// 是否腾空
bool motor_airborne_detector_is_airborne(const motor_airborne_detector_t* detector) {
  return detector ? detector->is_airborne : false;
}

// 校准基线电流
void motor_airborne_detector_calibrate(motor_airborne_detector_t* detector,
  float ground_current) {
  if (!detector) return;

  detector->baseline_current = ground_current;
}
