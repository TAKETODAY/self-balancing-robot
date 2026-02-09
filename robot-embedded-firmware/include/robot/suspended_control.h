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

#ifndef SUSPENDED_CONTROL_H
#define SUSPENDED_CONTROL_H

#include "multi_sensor_suspended.h"

// 悬空控制模式
typedef enum {
  SUSPENDED_CTRL_NONE = 0, // 无控制
  SUSPENDED_CTRL_STABILIZE, // 姿态稳定
  SUSPENDED_CTRL_DESCEND, // 缓慢下降
  SUSPENDED_CTRL_RECOVER, // 恢复尝试
  SUSPENDED_CTRL_EMERGENCY // 紧急处理
} suspended_control_mode_t;

// 悬空控制器
typedef struct {
  suspended_control_mode_t mode; // 当前控制模式
  multi_sensor_detector_t detector; // 悬空检测器

  // 控制参数
  struct {
    float stabilize_kp; // 稳定控制P
    float stabilize_kd; // 稳定控制D
    float descend_rate; // 下降速率
    float recover_power; // 恢复功率
    float emergency_stop_time; // 急停时间
  } params;

  // 状态
  struct {
    uint32_t mode_start_time; // 模式开始时间
    uint32_t total_suspended_time; // 总悬空时间
    uint32_t max_suspended_time; // 最长悬空时间
    uint32_t recovery_attempts; // 恢复尝试次数
    bool recovery_successful; // 恢复是否成功
  } status;

  // 安全限制
  struct {
    uint32_t max_safe_time; // 最大安全时间
    float max_tilt_angle; // 最大倾斜角度
    float max_descent_speed; // 最大下降速度
    bool enable_auto_shutdown; // 启用自动关机
  } safety;
} suspended_controller_t;

// API
void suspended_controller_init(suspended_controller_t* controller);
void suspended_controller_update(suspended_controller_t* controller,
  const sensor_data_t* sensors,
  uint8_t sensor_count);
suspended_control_mode_t suspended_controller_get_mode(const suspended_controller_t* controller);
void suspended_controller_calculate_output(const suspended_controller_t* controller,
  float* motor_left,
  float* motor_right);
void suspended_controller_print_status(const suspended_controller_t* controller);

#endif // SUSPENDED_CONTROL_H
