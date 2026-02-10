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

#ifndef ATTITUDE_TO_SUSPENDED_ADAPTER_H
#define ATTITUDE_TO_SUSPENDED_ADAPTER_H

#include "attitude_sensor.h"
#include "sustained_airborne.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// MPU6050数据适配器
typedef struct {
  suspended_detector_t suspended_detector; // 悬空检测器
  bool is_initialized;                     // 是否已初始化

  // 转换参数
  float accel_scale; // 加速度计比例因子 (根据MPU6050量程设置)
  float gyro_scale;  // 陀螺仪比例因子

  // 历史数据用于滤波
  struct {
    float vertical_accel_buffer[10];  // 垂直加速度缓冲区
    uint8_t buffer_index;             // 缓冲区索引
    float filtered_vertical_accel;    // 滤波后的垂直加速度
    float filtered_vertical_velocity; // 滤波后的垂直速度
  } filter;

  // 校准数据
  struct {
    float ground_accel_z;  // 地面时Z轴加速度
    float ground_variance; // 地面加速度方差
    bool calibrated;       // 是否已校准
  } calibration;
} mpu6050_suspended_adapter_t;

// ========== API 函数 ==========
// 初始化适配器
bool mpu6050_suspended_adapter_init(mpu6050_suspended_adapter_t* adapter,
  float accel_scale, float gyro_scale);

// 校准适配器（在地面静止时调用）
bool mpu6050_suspended_adapter_calibrate(mpu6050_suspended_adapter_t* adapter,
  uint16_t calibration_time_ms);

// 更新悬空检测
suspended_state_t mpu6050_suspended_adapter_update(mpu6050_suspended_adapter_t* adapter);

// 获取当前悬空状态
suspended_state_t mpu6050_suspended_adapter_get_state(const mpu6050_suspended_adapter_t* adapter);

// 检查是否持续悬空
bool mpu6050_suspended_adapter_is_suspended(const mpu6050_suspended_adapter_t* adapter);

// 获取悬空持续时间 (ms)
uint32_t mpu6050_suspended_adapter_get_duration(const mpu6050_suspended_adapter_t* adapter);

// 获取检测置信度
float mpu6050_suspended_adapter_get_confidence(const mpu6050_suspended_adapter_t* adapter);

// 获取估计高度 (m)
float mpu6050_suspended_adapter_get_estimated_height(const mpu6050_suspended_adapter_t* adapter);

// 重置适配器
void mpu6050_suspended_adapter_reset(mpu6050_suspended_adapter_t* adapter);

// 打印状态
void mpu6050_suspended_adapter_print_status(const mpu6050_suspended_adapter_t* adapter);


#ifdef __cplusplus
}
#endif


#endif // ATTITUDE_TO_SUSPENDED_ADAPTER_H
