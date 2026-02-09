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

#ifndef MULTI_SENSOR_SUSPENDED_H
#define MULTI_SENSOR_SUSPENDED_H

#include "sustained_airborne.h"

// 传感器类型
typedef enum {
  SENSOR_IMU = 0, // 惯性测量单元
  SENSOR_MOTOR_CURRENT, // 电机电流
  SENSOR_ENCODER, // 编码器
  SENSOR_VIBRATION, // 振动传感器
  SENSOR_ULTRASONIC, // 超声波
  SENSOR_PRESSURE, // 压力传感器
} sensor_type_t;

// 传感器数据
typedef struct {
  sensor_type_t type; // 传感器类型

  union {
    struct {
      float accel[3]; // 加速度
      float gyro[3]; // 角速度
    } imu;

    struct {
      float left; // 左电机电流
      float right; // 右电机电流
      float load; // 负载百分比
    } motor;

    struct {
      float speed; // 转速
      float slip; // 滑移率
    } encoder;

    struct {
      float amplitude; // 振幅
      float frequency; // 频率
    } vibration;

    struct {
      float distance; // 距离
      float confidence; // 置信度
    } ultrasonic;

    struct {
      float force; // 压力
      float variance; // 方差
    } pressure;
  } data;

  uint32_t timestamp; // 时间戳
  float reliability; // 数据可靠性 (0.0-1.0)
} sensor_data_t;

// 多传感器融合检测器
typedef struct {
  suspended_detector_t imu_detector; // IMU检测器

  // 其他传感器状态
  struct {
    bool motor_off_ground; // 电机离地状态
    bool encoder_off_ground; // 编码器离地状态
    bool vibration_off_ground; // 振动离地状态
    bool ultrasonic_off_ground; // 超声波离地状态
    bool pressure_off_ground; // 压力离地状态
  } sensor_states;

  // 融合权重
  struct {
    float imu_weight; // IMU权重
    float motor_weight; // 电机权重
    float encoder_weight; // 编码器权重
    float vibration_weight; // 振动权重
    float ultrasonic_weight; // 超声波权重
    float pressure_weight; // 压力权重
  } weights;

  // 融合结果
  suspended_state_t fused_state; // 融合状态
  float fused_confidence; // 融合置信度
  bool is_definitely_suspended; // 确定悬空
  bool is_definitely_grounded; // 确定接地

  // 校准状态
  struct {
    float ground_current; // 地面电流基线
    float ground_vibration; // 地面振动基线
    float ground_distance; // 地面距离基线
    bool calibrated[6]; // 各传感器校准状态
  } calibration;

  // 统计
  uint32_t detection_count; // 检测次数
  uint32_t correct_detections; // 正确检测
  uint32_t false_detections; // 错误检测
} multi_sensor_detector_t;

// API
void multi_sensor_detector_init(multi_sensor_detector_t* detector);
void multi_sensor_detector_calibrate(multi_sensor_detector_t* detector,
  const sensor_data_t* sensor_data);
suspended_state_t multi_sensor_detector_update(multi_sensor_detector_t* detector,
  const sensor_data_t* sensors,
  uint8_t sensor_count);
bool multi_sensor_is_suspended(const multi_sensor_detector_t* detector);
void multi_sensor_print_status(const multi_sensor_detector_t* detector);

#endif // MULTI_SENSOR_SUSPENDED_H
