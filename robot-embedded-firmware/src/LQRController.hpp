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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "defs.h"

class LQRController {

public:
  LQRController() = default;
  ~LQRController() = default;

  void begin();

  void resetZeroPoint();
  void balance_loop();
  void yaw_loop();

  void stop();
  void start();

  bool is_started();

private:
  TaskHandle_t task_handle = nullptr;

public:
  // LQR自平衡控制器参数
  float LQR_angle = 0;
  float LQR_gyro = 0;
  float LQR_gyroZ = 0;
  float LQR_speed = 0;
  float LQR_distance = 0;
  float angle_control = 0;
  float gyro_control = 0;
  float speed_control = 0;
  float distance_control = 0;
  float LQR_u = 0;
  float pitch_zeropoint = 2;                        // 7 设定默认俯仰角度，向前到正，向后倒负
  float original_pitch_zeropoint = pitch_zeropoint; // 保存原始的角度零点
  float distance_zeropoint = 0.5f;                  // 轮部位移零点偏置
  float pitch_adjust = 0.0f;                        // 俯仰角度调整,负数前倾，正数后倾

  // 记录轮部转速，用于判断跳跃状态
  unsigned long last_speed_record_time = 0;        // 上次记录转速的时间
  const unsigned long SPEED_RECORD_INTERVAL = 100; // 转速记录间隔(毫秒) ，因为时间相隔太近速度差不明显的

  float last_lqr_speed = 0;   // 记录上一时刻的轮部转速
  float robot_speed_diff = 0; // 速度差

  // YAW轴控制数据
  float YAW_gyro = 0;
  float YAW_angle = 0;
  float YAW_angle_last = 0;
  float YAW_angle_total = 0;
  float YAW_angle_zero_point = -10;
  float YAW_output = 0;

  // 跳跃相关参数
  int jump_flag = 0; // 跳跃过程计数

};
