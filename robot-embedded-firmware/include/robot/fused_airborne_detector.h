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

// fused_airborne_detector.h
#ifndef FUSED_AIRBORNE_DETECTOR_H
#define FUSED_AIRBORNE_DETECTOR_H

#include "airborne_detector.h"
#include "motor_airborne_detector.h"

// 融合权重
typedef struct {
  float imu_weight; // IMU检测权重 (0.0-1.0)
  float motor_weight; // 电机检测权重 (0.0-1.0)
  float vibration_weight; // 振动检测权重
  float confidence_weight; // 置信度权重
} fusion_weights_t;

// 融合检测器
typedef struct {
  airborne_detector_t imu_detector; // IMU检测器
  motor_airborne_detector_t left_motor_detector; // 左电机检测器
  motor_airborne_detector_t right_motor_detector; // 右电机检测器

  fusion_weights_t weights; // 融合权重

  // 融合结果
  bool is_airborne; // 最终腾空判断
  float fused_confidence; // 融合置信度
  airborne_state_t fused_state; // 融合状态

  // 校准状态
  bool is_calibrated; // 是否已校准
  float calibration_ground_level; // 校准地面水平
  uint32_t calibration_time; // 校准时间

  // 统计
  uint32_t false_positives; // 误报次数
  uint32_t false_negatives; // 漏报次数
  uint32_t correct_detections; // 正确检测次数
} fused_airborne_detector_t;

// API
void fused_airborne_detector_init(fused_airborne_detector_t* detector);
void fused_airborne_detector_calibrate(fused_airborne_detector_t* detector,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor);
bool fused_airborne_detector_update(fused_airborne_detector_t* detector,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor);
bool fused_airborne_detector_is_airborne(const fused_airborne_detector_t* detector);
float fused_airborne_detector_get_confidence(const fused_airborne_detector_t* detector);
void fused_airborne_detector_print_stats(const fused_airborne_detector_t* detector);

#endif // FUSED_AIRBORNE_DETECTOR_H
