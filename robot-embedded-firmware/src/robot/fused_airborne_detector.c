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

#include "robot/fused_airborne_detector.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// 默认权重
static const fusion_weights_t DEFAULT_WEIGHTS = {
  .imu_weight = 0.7f,
  .motor_weight = 0.3f,
  .vibration_weight = 0.5f,
  .confidence_weight = 0.8f
};

// 初始化
void fused_airborne_detector_init(fused_airborne_detector_t* detector) {
  if (!detector)
    return;

  memset(detector, 0, sizeof(fused_airborne_detector_t));

  // 初始化子检测器
  airborne_detector_init(&detector->imu_detector, NULL);
  motor_airborne_detector_init(&detector->left_motor_detector);
  motor_airborne_detector_init(&detector->right_motor_detector);

  // 设置权重
  detector->weights = DEFAULT_WEIGHTS;

  detector->is_calibrated = false;
}

// 计算向量幅值
static inline float vector_magnitude(const float v[3]) {
  return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

// 校准（在地面静止时调用）
void fused_airborne_detector_calibrate(fused_airborne_detector_t* detector,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor) {
  if (!detector || !imu || !left_motor || !right_motor) return;

  // 校准IMU检测器
  detector->imu_detector.config.gravity = vector_magnitude(imu->accel);

  // 校准电机检测器
  float avg_current = (fabsf(left_motor->current) + fabsf(right_motor->current)) / 2.0f;
  motor_airborne_detector_calibrate(&detector->left_motor_detector, avg_current);
  motor_airborne_detector_calibrate(&detector->right_motor_detector, avg_current);

  // 保存校准地面水平
  detector->calibration_ground_level = imu->accel[2]; // Z轴加速度
  detector->calibration_time = imu->timestamp;

  detector->is_calibrated = true;

  printf("融合检测器校准完成\n");
  printf("重力加速度: %.2f m/s²\n", detector->imu_detector.config.gravity);
  printf("基线电流: %.2f A\n", avg_current);
}

// 更新融合检测
bool fused_airborne_detector_update(fused_airborne_detector_t* detector,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor) {
  if (!detector || !imu) return false;

  // 更新各个检测器
  airborne_state_t imu_state = airborne_detector_update(&detector->imu_detector, imu);
  float imu_confidence = airborne_detector_get_confidence(&detector->imu_detector);

  bool left_motor_airborne = false;
  bool right_motor_airborne = false;

  if (left_motor) {
    left_motor_airborne = motor_airborne_detector_update(&detector->left_motor_detector,
      left_motor);
  }

  if (right_motor) {
    right_motor_airborne = motor_airborne_detector_update(&detector->right_motor_detector,
      right_motor);
  }

  bool motor_airborne = left_motor_airborne && right_motor_airborne;

  // 融合决策
  float motor_score = motor_airborne ? 1.0f : 0.0f;
  float imu_score = (imu_state == WHEELS_OFF_GROUND || imu_state == FULLY_AIRBORNE) ? 1.0f : 0.0f;

  // 加权融合
  float imu_weighted = imu_score * detector->weights.imu_weight;
  float motor_weighted = motor_score * detector->weights.motor_weight;
  float confidence_weighted = imu_confidence * detector->weights.confidence_weight;

  detector->fused_confidence = (imu_weighted + motor_weighted + confidence_weighted) / 3.0f;

  // 阈值判断
  float detection_threshold = 0.6f; // 60%置信度阈值
  detector->is_airborne = detector->fused_confidence > detection_threshold;

  // 确定融合状态
  if (detector->is_airborne) {
    if (imu_state == FULLY_AIRBORNE && detector->fused_confidence > 0.8f) {
      detector->fused_state = FULLY_AIRBORNE;
    }
    else {
      detector->fused_state = WHEELS_OFF_GROUND;
    }
  }
  else {
    if (imu_state == LANDING_IMPACT) {
      detector->fused_state = LANDING_IMPACT;
    }
    else {
      detector->fused_state = GROUND_CONTACT;
    }
  }

  // 统计
  bool true_airborne = false; // 这里需要实际的真值（可通过其他方法获取）
  if (detector->is_airborne == true_airborne) {
    detector->correct_detections++;
  }
  else if (detector->is_airborne && !true_airborne) {
    detector->false_positives++;
  }
  else if (!detector->is_airborne && true_airborne) {
    detector->false_negatives++;
  }

  return detector->is_airborne;
}

// 是否腾空
bool fused_airborne_detector_is_airborne(const fused_airborne_detector_t* detector) {
  return detector ? detector->is_airborne : false;
}

// 获取置信度
float fused_airborne_detector_get_confidence(const fused_airborne_detector_t* detector) {
  return detector ? detector->fused_confidence : 0.0f;
}

// 打印统计
void fused_airborne_detector_print_stats(const fused_airborne_detector_t* detector) {
  if (!detector) return;

  uint32_t total = detector->correct_detections +
                   detector->false_positives +
                   detector->false_negatives;

  if (total == 0) return;

  float accuracy = (float) detector->correct_detections / total * 100.0f;
  float fp_rate = (float) detector->false_positives / total * 100.0f;
  float fn_rate = (float) detector->false_negatives / total * 100.0f;

  printf("==== 腾空检测统计 ====\n");
  printf("总检测次数: %lu\n", total);
  printf("正确检测: %lu (%.1f%%)\n", detector->correct_detections, accuracy);
  printf("误报次数: %lu (%.1f%%)\n", detector->false_positives, fp_rate);
  printf("漏报次数: %lu (%.1f%%)\n", detector->false_negatives, fn_rate);
  printf("融合置信度: %.1f%%\n", detector->fused_confidence * 100);
  printf("====================\n\n");
}
