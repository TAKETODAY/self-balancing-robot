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

// airborne_detector.c

#include "robot/airborne_detector.h"
#include <string.h>
#include <stdio.h>

// 默认配置
static const airborne_config_t DEFAULT_CONFIG = {
  .algorithm = DETECT_COMBINED,
  .accel_threshold = 0.15f, // 0.15g阈值
  .gyro_threshold = 0.5f, // 0.5 rad/s阈值
  .variance_threshold = 0.05f, // 方差阈值
  .impact_threshold = 2.5f, // 2.5g冲击阈值
  .min_airborne_time = 30, // 最小30ms腾空时间
  .max_airborne_time = 5000, // 最大5秒腾空
  .confidence_threshold = 0.7f, // 70%置信度
  .gravity = 9.80665f // 标准重力
};

// 初始化
void airborne_detector_init(airborne_detector_t* detector, const airborne_config_t* config) {
  if (!detector) return;

  memset(detector, 0, sizeof(airborne_detector_t));

  if (config) {
    detector->config = *config;
  }
  else {
    detector->config = DEFAULT_CONFIG;
  }

  detector->state = GROUND_CONTACT;
  detector->accel_filter_alpha = 0.2f;
  detector->gyro_filter_alpha = 0.2f;
  detector->free_fall_accel = detector->config.gravity * 0.1f; // 0.1g作为自由落体阈值

  // 初始化滤波器
  for (int i = 0; i < 3; i++) {
    detector->filtered_accel[i] = 0;
    detector->filtered_gyro[i] = 0;
  }

  detector->history_index = 0;
}

// 低通滤波器
static inline float low_pass_filter(float new_val, float old_val, float alpha) {
  return old_val * (1.0f - alpha) + new_val * alpha;
}

// 计算向量幅值
static inline float vector_magnitude(const float v[3]) {
  return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

// 计算方差
static float calculate_variance(const float history[][3], int count, int axis) {
  if (count <= 1) return 0.0f;

  float sum = 0.0f;
  float sum_sq = 0.0f;

  for (int i = 0; i < count; i++) {
    sum += history[i][axis];
    sum_sq += history[i][axis] * history[i][axis];
  }

  float mean = sum / count;
  return (sum_sq / count) - (mean * mean);
}

// 基于加速度的检测
static bool detect_by_acceleration(airborne_detector_t* detector) {
  // 垂直加速度接近重力（没有地面反作用力）
  float vertical_diff = fabsf(detector->vertical_acceleration - detector->config.gravity);

  // 合加速度接近重力
  float total_diff = fabsf(detector->total_acceleration - detector->config.gravity);

  // 低加速度变化（没有地面振动）
  bool low_variance = (detector->acceleration_variance < detector->config.variance_threshold);

  // 检测自由落体（垂直加速度接近0）
  bool free_fall = (fabsf(detector->vertical_acceleration) < detector->free_fall_accel);

  return (vertical_diff < detector->config.accel_threshold) &&
         low_variance &&
         !free_fall;
}

// 基于振动的检测
static bool detect_by_vibration(airborne_detector_t* detector) {
  // 腾空时振动显著减小
  bool low_accel_variance = (detector->acceleration_variance < detector->config.variance_threshold * 0.5f);
  bool low_gyro_variance = (detector->gyro_variance < detector->config.gyro_threshold * 0.5f);

  return low_accel_variance && low_gyro_variance;
}

// 综合检测算法
static bool detect_combined(airborne_detector_t* detector) {
  float accel_score = 0.0f;
  float vib_score = 0.0f;

  // 加速度检测得分
  float vertical_diff = fabsf(detector->vertical_acceleration - detector->config.gravity);
  accel_score = 1.0f - fminf(vertical_diff / detector->config.accel_threshold, 1.0f);

  // 振动检测得分
  float var_ratio = detector->acceleration_variance / detector->config.variance_threshold;
  vib_score = 1.0f - fminf(var_ratio, 1.0f);

  // 角速度得分
  float gyro_mag = vector_magnitude(detector->filtered_gyro);
  float gyro_score = 1.0f - fminf(gyro_mag / detector->config.gyro_threshold, 1.0f);

  // 加权综合得分
  detector->detection_confidence = (accel_score * 0.4f + vib_score * 0.4f + gyro_score * 0.2f);

  return detector->detection_confidence > detector->config.confidence_threshold;
}

// 检测着陆冲击
static bool detect_landing_impact(airborne_detector_t* detector) {
  // 垂直加速度的快速变化
  float accel_diff = fabsf(detector->vertical_acceleration - detector->last_vertical_accel);
  bool high_accel_change = (accel_diff > detector->config.impact_threshold);

  // 绝对加速度值大
  bool high_vertical_accel = (fabsf(detector->vertical_acceleration) > detector->config.gravity * 1.5f);

  return high_accel_change && high_vertical_accel;
}

// 更新检测器
airborne_state_t airborne_detector_update(airborne_detector_t* detector, const imu_data_t* imu) {
  if (!detector || !imu) return UNCERTAIN_STATE;

  // 更新时间
  uint32_t current_time = imu->timestamp;
  float dt = (current_time - detector->last_update_time) / 1000.0f; // 转换为秒
  if (dt <= 0) dt = 0.001f;
  detector->last_update_time = current_time;

  // 滤波处理
  for (int i = 0; i < 3; i++) {
    detector->filtered_accel[i] = low_pass_filter(
      imu->accel[i],
      detector->filtered_accel[i],
      detector->accel_filter_alpha
    );
    detector->filtered_gyro[i] = low_pass_filter(
      imu->gyro[i],
      detector->filtered_gyro[i],
      detector->gyro_filter_alpha
    );
  }

  // 保存历史数据
  for (int i = 0; i < 3; i++) {
    detector->accel_history[detector->history_index][i] = detector->filtered_accel[i];
  }
  detector->history_index = (detector->history_index + 1) % 20;

  // 计算关键指标
  detector->vertical_acceleration = detector->filtered_accel[2]; // 假设Z轴垂直向上
  detector->total_acceleration = vector_magnitude(detector->filtered_accel);

  // 计算方差（使用最近10个样本）
  detector->acceleration_variance = calculate_variance(detector->accel_history, 10, 2);

  // 更新最大值
  if (fabsf(detector->vertical_acceleration) > detector->max_vertical_accel) {
    detector->max_vertical_accel = fabsf(detector->vertical_acceleration);
  }

  // 状态机
  airborne_state_t old_state = detector->state;
  bool is_airborne = false;

  // 根据算法选择检测方法
  switch (detector->config.algorithm) {
    case DETECT_ACCELERATION:
      is_airborne = detect_by_acceleration(detector);
      break;
    case DETECT_VIBRATION:
      is_airborne = detect_by_vibration(detector);
      break;
    case DETECT_COMBINED:
    case DETECT_ADAPTIVE:
    default:
      is_airborne = detect_combined(detector);
      break;
  }

  // 检测着陆冲击
  bool landing_impact = detect_landing_impact(detector);

  // 状态转换逻辑
  if (is_airborne) {
    if (detector->state == GROUND_CONTACT) {
      // 刚离地
      detector->state = WHEELS_OFF_GROUND;
      detector->airborne_start_time = current_time;
      detector->airborne_count++;
    }
    else if (detector->state == WHEELS_OFF_GROUND) {
      // 持续腾空
      uint32_t airborne_time = current_time - detector->airborne_start_time;
      if (airborne_time > detector->config.min_airborne_time) {
        detector->state = FULLY_AIRBORNE;
        detector->total_airborne_time += airborne_time;
      }
    }
  }
  else {
    if (landing_impact && (detector->state == WHEELS_OFF_GROUND ||
                           detector->state == FULLY_AIRBORNE)) {
      // 着陆冲击
      detector->state = LANDING_IMPACT;
      detector->max_impact_force = fabsf(detector->vertical_acceleration);

      // 短延时后回到地面状态
      detector->state_timestamp = current_time;
    }
    else if (detector->state == LANDING_IMPACT) {
      // 着陆冲击后恢复
      if (current_time - detector->state_timestamp > 100) {
        // 100ms后恢复
        detector->state = GROUND_CONTACT;
      }
    }
    else {
      detector->state = GROUND_CONTACT;
    }
  }

  // 保存当前垂直加速度用于下次比较
  detector->last_vertical_accel = detector->vertical_acceleration;

  // 状态变化回调（可以在这里添加）
  if (old_state != detector->state) {
    detector->state_timestamp = current_time;
  }

  return detector->state;
}

// 获取当前状态
airborne_state_t airborne_detector_get_state(const airborne_detector_t* detector) {
  return detector ? detector->state : UNCERTAIN_STATE;
}

// 获取腾空时间
uint32_t airborne_detector_get_airborne_duration(const airborne_detector_t* detector) {
  if (!detector || detector->state == GROUND_CONTACT) {
    return 0;
  }
  return detector->last_update_time - detector->airborne_start_time;
}

// 获取置信度
float airborne_detector_get_confidence(const airborne_detector_t* detector) {
  return detector ? detector->detection_confidence : 0.0f;
}

// 检查是否腾空
bool airborne_detector_is_airborne(const airborne_detector_t* detector) {
  return detector && (detector->state == WHEELS_OFF_GROUND ||
                      detector->state == FULLY_AIRBORNE);
}

// 检查是否正在着陆
bool airborne_detector_is_landing(const airborne_detector_t* detector) {
  return detector && (detector->state == LANDING_IMPACT);
}

// 获取垂直加速度
float airborne_detector_get_vertical_accel(const airborne_detector_t* detector) {
  return detector ? detector->vertical_acceleration : 0.0f;
}

// 估计高度（简化的积分方法）
float airborne_detector_estimate_height(const airborne_detector_t* detector) {
  if (!detector || detector->state == GROUND_CONTACT) {
    return 0.0f;
  }

  // 简化的垂直速度积分
  static float vertical_velocity = 0.0f;
  static float estimated_height = 0.0f;

  // 垂直加速度（减去重力）
  float vertical_accel = detector->vertical_acceleration - detector->config.gravity;

  // 假设10ms更新周期
  float dt = 0.01f;

  // 积分得到速度
  vertical_velocity += vertical_accel * dt;

  // 积分得到高度
  estimated_height += vertical_velocity * dt;

  // 限制和修正
  if (estimated_height < 0) estimated_height = 0;
  if (estimated_height > 1.0f) estimated_height = 1.0f; // 最大1米

  return estimated_height;
}

// 重置检测器
void airborne_detector_reset(airborne_detector_t* detector) {
  if (!detector) return;

  detector->state = GROUND_CONTACT;
  detector->airborne_start_time = 0;
  detector->vertical_acceleration = 0;
  detector->total_acceleration = detector->config.gravity;
  detector->last_vertical_accel = 0;
  detector->detection_confidence = 0.0f;
}

// 打印状态
void airborne_detector_print_status(const airborne_detector_t* detector) {
  if (!detector) return;

  const char* state_str[] = {
    "地面接触", "轮子离地", "完全腾空", "着陆冲击", "不确定"
  };

  printf("腾空状态: %s\n", state_str[detector->state]);
  printf("垂直加速度: %.2f m/s²\n", detector->vertical_acceleration);
  printf("合加速度: %.2f m/s²\n", detector->total_acceleration);
  printf("加速度方差: %.4f\n", detector->acceleration_variance);
  printf("检测置信度: %.1f%%\n", detector->detection_confidence * 100);
  printf("腾空次数: %lu\n", detector->airborne_count);

  if (detector->state != GROUND_CONTACT) {
    uint32_t duration = airborne_detector_get_airborne_duration(detector);
    printf("腾空时间: %lu ms\n", duration);

    float height = airborne_detector_estimate_height(detector);
    printf("估计高度: %.2f m\n", height);
  }
  printf("\n");
}
