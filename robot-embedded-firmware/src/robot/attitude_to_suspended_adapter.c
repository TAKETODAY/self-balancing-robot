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

#include "robot/attitude_to_suspended_adapter.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 日志标签
static const char* TAG = "MPU6050_ADAPTER";

// 初始化适配器
bool mpu6050_suspended_adapter_init(mpu6050_suspended_adapter_t* adapter,
  float accel_scale,
  float gyro_scale) {
  if (!adapter) {
    ESP_LOGE(TAG, "无效的参数");
    return false;
  }

  memset(adapter, 0, sizeof(mpu6050_suspended_adapter_t));

  adapter->accel_scale = accel_scale;
  adapter->gyro_scale = gyro_scale;

  // 初始化悬空检测器
  suspended_config_t config = {
    .algorithm = DETECTION_ADAPTIVE,
    .transient_threshold = 100,
    .stable_threshold = 1000,
    .long_term_threshold = 5000,
    .pattern_similarity = 0.85f,
    .pattern_window = 50,
    .vibration_frequency = 5.0f,
    .frequency_stability = 0.3f,
    .adaptation_rate = 0.1f,
    .adaptation_interval = 5000,
    .enable_auto_recovery = true,
    .max_suspended_time = 30000,
    .recovery_confidence = 0.7f
  };

  suspended_detector_init(&adapter->suspended_detector, &config);

  // 初始化滤波器
  for (int i = 0; i < 10; i++) {
    adapter->filter.vertical_accel_buffer[i] = 0.0f;
  }

  adapter->is_initialized = true;

  ESP_LOGI(TAG, "MPU6050悬空检测适配器初始化完成");
  return true;
}

// 低通滤波器
static float low_pass_filter(float new_value, float old_value, float alpha) {
  return old_value * (1.0f - alpha) + new_value * alpha;
}

// 计算垂直加速度
static float calculate_vertical_acceleration(float accel_scale) {
  // 将MPU6050原始数据转换为m/s²
  // MPU6050加速度计默认量程±2g时，灵敏度为16384 LSB/g
  const mpu6050_axis_value_t* acce = attitude_get_acceleration();
  const float ax = acce->x / 16384.0f * 9.80665f * accel_scale;
  const float ay = acce->y / 16384.0f * 9.80665f * accel_scale;
  const float az = acce->z / 16384.0f * 9.80665f * accel_scale;

  // 使用姿态角将加速度从机器人坐标系转换到世界坐标系
  // roll和pitch应从姿态传感器获取（单位为弧度）
  const float roll = attitude_get_roll() * M_PI / 180.0f;   // 转换为弧度
  const float pitch = attitude_get_pitch() * M_PI / 180.0f; // 转换为弧度

  // 旋转矩阵计算垂直加速度
  // R = Rz(yaw) * Ry(pitch) * Rx(roll)
  // 这里简化：只考虑pitch和roll的影响
  float cos_roll = cosf(roll);
  float sin_roll = sinf(roll);
  float cos_pitch = cosf(pitch);
  float sin_pitch = sinf(pitch);

  // 世界坐标系下的垂直加速度
  // 注意：这里假设传感器Z轴向上，需要根据实际安装方向调整
  float vertical_accel =
      -ax * sin_pitch +
      ay * sin_roll * cos_pitch +
      az * cos_roll * cos_pitch;

  return vertical_accel;
}

// 计算垂直速度（通过加速度积分）
static float calculate_vertical_velocity(mpu6050_suspended_adapter_t* adapter, float vertical_accel, float dt) {
  static float last_vertical_velocity = 0.0f;

  if (dt <= 0) dt = 0.001f; // 最小时间间隔

  // 减去重力加速度（9.80665 m/s²）
  float motion_accel = vertical_accel - 9.80665f;

  // 积分得到速度（简单的欧拉积分）
  float vertical_velocity = last_vertical_velocity + motion_accel * dt;

  // 应用低通滤波减少噪声
  vertical_velocity = low_pass_filter(vertical_velocity, last_vertical_velocity, 0.2f);

  // 限制速度范围
  if (vertical_velocity > 5.0f) vertical_velocity = 5.0f;
  if (vertical_velocity < -5.0f) vertical_velocity = -5.0f;

  last_vertical_velocity = vertical_velocity;
  return vertical_velocity;
}

// 校准适配器
bool mpu6050_suspended_adapter_calibrate(mpu6050_suspended_adapter_t* adapter, uint16_t calibration_time_ms) {
  if (!adapter || !adapter->is_initialized) {
    ESP_LOGE(TAG, "适配器未初始化");
    return false;
  }

  ESP_LOGI(TAG, "开始校准，请保持机器人在地面静止...");

  // 等待机器人静止
  vTaskDelay(pdMS_TO_TICKS(1000));

  // 采集校准数据
  const int samples = 100;
  float accel_sum = 0.0f;
  float accel_sq_sum = 0.0f;

  for (int i = 0; i < samples; i++) {
    // 更新传感器数据
    attitude_update();

    // 计算垂直加速度
    float vertical_accel = calculate_vertical_acceleration(adapter->accel_scale);

    accel_sum += vertical_accel;
    accel_sq_sum += vertical_accel * vertical_accel;

    vTaskDelay(pdMS_TO_TICKS(10)); // 10ms间隔
  }

  // 计算平均值和方差
  float mean = accel_sum / samples;
  float variance = (accel_sq_sum / samples) - (mean * mean);

  // 保存校准数据
  adapter->calibration.ground_accel_z = mean;
  adapter->calibration.ground_variance = variance;
  adapter->calibration.calibrated = true;

  // 校准悬空检测器的地面模式
  float ground_samples[50];
  for (int i = 0; i < 50; i++) {
    ground_samples[i] = mean;
  }

  suspended_detector_calibrate_ground(&adapter->suspended_detector, ground_samples, 50);

  ESP_LOGI(TAG, "校准完成: 地面加速度=%.2f m/s², 方差=%.4f", mean, variance);

  return true;
}

// 更新悬空检测
suspended_state_t mpu6050_suspended_adapter_update(mpu6050_suspended_adapter_t* adapter) {
  if (!adapter || !adapter->is_initialized) {
    return SUSPENDED_NONE;
  }

  static uint32_t last_timestamp = 0;
  uint32_t current_time = esp_timer_get_time() / 1000; // 转换为ms

  // 更新时间间隔
  float dt = (current_time - last_timestamp) / 1000.0f; // 转换为秒
  if (last_timestamp == 0 || dt <= 0) {
    dt = 0.001f;
  }
  last_timestamp = current_time;

  // 更新MPU6050数据
  attitude_update();

  // 计算垂直加速度
  float raw_vertical_accel = calculate_vertical_acceleration(adapter->accel_scale);

  // 更新滤波器缓冲区
  adapter->filter.vertical_accel_buffer[adapter->filter.buffer_index] = raw_vertical_accel;
  adapter->filter.buffer_index = (adapter->filter.buffer_index + 1) % 10;

  // 计算平均值（简单的移动平均）
  float sum = 0.0f;
  for (int i = 0; i < 10; i++) {
    sum += adapter->filter.vertical_accel_buffer[i];
  }

  float avg_vertical_accel = sum / 10.0f;

  // 应用低通滤波
  adapter->filter.filtered_vertical_accel = low_pass_filter(avg_vertical_accel, adapter->filter.filtered_vertical_accel, 0.3f);

  // 计算垂直速度
  adapter->filter.filtered_vertical_velocity = calculate_vertical_velocity(adapter, adapter->filter.filtered_vertical_accel, dt);

  // 更新悬空检测器
  return suspended_detector_update(&adapter->suspended_detector,
    adapter->filter.filtered_vertical_accel, adapter->filter.filtered_vertical_velocity, current_time);
}

// 获取当前悬空状态
suspended_state_t mpu6050_suspended_adapter_get_state(const mpu6050_suspended_adapter_t* adapter) {
  return adapter ? suspended_detector_get_state(&adapter->suspended_detector) : SUSPENDED_NONE;
}

// 检查是否持续悬空
bool mpu6050_suspended_adapter_is_suspended(const mpu6050_suspended_adapter_t* adapter) {
  if (!adapter) return false;
  return suspended_detector_is_suspended(&adapter->suspended_detector);
}

// 获取悬空持续时间
uint32_t mpu6050_suspended_adapter_get_duration(const mpu6050_suspended_adapter_t* adapter) {
  return adapter ? suspended_detector_get_duration(&adapter->suspended_detector) : 0;
}

// 获取检测置信度
float mpu6050_suspended_adapter_get_confidence(const mpu6050_suspended_adapter_t* adapter) {
  return adapter ? suspended_detector_get_confidence(&adapter->suspended_detector) : 0.0f;
}

// 获取估计高度
float mpu6050_suspended_adapter_get_estimated_height(const mpu6050_suspended_adapter_t* adapter) {
  if (!adapter) return 0.0f;

  // 简化的垂直速度积分估算高度
  static float estimated_height = 0.0f;
  static uint32_t last_update_time = 0;

  uint32_t current_time = esp_timer_get_time() / 1000;
  float dt = (current_time - last_update_time) / 1000.0f;

  if (last_update_time == 0 || dt <= 0) {
    dt = 0.001f;
  }

  // 只在悬空时积分高度
  if (mpu6050_suspended_adapter_is_suspended(adapter)) {
    estimated_height += adapter->filter.filtered_vertical_velocity * dt;
  }
  else {
    // 接地时重置高度
    estimated_height = 0.0f;
  }

  // 限制高度范围
  if (estimated_height < 0) estimated_height = 0.0f;
  if (estimated_height > 2.0f) estimated_height = 2.0f; // 最大2米

  last_update_time = current_time;
  return estimated_height;
}

// 重置适配器
void mpu6050_suspended_adapter_reset(mpu6050_suspended_adapter_t* adapter) {
  if (!adapter) return;

  suspended_detector_reset(&adapter->suspended_detector);

  // 重置滤波器
  for (int i = 0; i < 10; i++) {
    adapter->filter.vertical_accel_buffer[i] = 0.0f;
  }
  adapter->filter.filtered_vertical_accel = 0.0f;
  adapter->filter.filtered_vertical_velocity = 0.0f;
  adapter->filter.buffer_index = 0;

  ESP_LOGI(TAG, "悬空检测适配器已重置");
}

// 打印状态
void mpu6050_suspended_adapter_print_status(const mpu6050_suspended_adapter_t* adapter) {
  if (!adapter) return;

  suspended_state_t state = mpu6050_suspended_adapter_get_state(adapter);
  uint32_t duration = mpu6050_suspended_adapter_get_duration(adapter);
  float confidence = mpu6050_suspended_adapter_get_confidence(adapter);
  float height = mpu6050_suspended_adapter_get_estimated_height(adapter);

  printf("=== MPU6050悬空检测状态 ===\n");
  printf("状态: %s\n", suspended_state_to_string(state));
  printf("持续时间: %lu ms\n", duration);
  printf("置信度: %.1f%%\n", confidence * 100);
  printf("估计高度: %.2f m\n", height);
  printf("垂直加速度: %.2f m/s²\n", adapter->filter.filtered_vertical_accel);
  printf("垂直速度: %.2f m/s\n", adapter->filter.filtered_vertical_velocity);

  printf("姿态角: Pitch=%.1f°, Roll=%.1f°, Yaw=%.1f°\n",
    attitude_get_pitch(), attitude_get_roll(), attitude_get_yaw());
  printf("==========================\n\n");
}
