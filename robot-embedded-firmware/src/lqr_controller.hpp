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

class lqr_controller {

public:
  lqr_controller() = default;
  ~lqr_controller() = default;

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

  int8_t joyx = 0; // 前后移动（-100~100）
  int8_t joyy = 0; // 左右转向（-100~100），正面看，正数：向左转，负数，向右转

  int8_t joyx_last = 0;
  int8_t joyy_last = 0;


  // 新增目标值
  float target_linear_speed; // 期望线速度（单位需与编码器一致）
  float target_yaw_rate;     // 期望偏航角速度（单位需与陀螺仪一致）

  // 新增接口：由外部设置左右轮指令
  void set_wheel_commands(float left, float right) {
    // 可在此处添加缩放、死区处理
    float left_scaled = left * 0.01f; // 假设摇杆范围-100~100，映射到实际速度
    float right_scaled = right * 0.01f;
    target_linear_speed = (left_scaled + right_scaled) * 0.5f;
    target_yaw_rate = (right_scaled - left_scaled) * TURN_GAIN; // TURN_GAIN 根据实测确定
  }

private:
  static constexpr float TURN_GAIN = 2.0f;   // 示例值，需实际标定
  static constexpr float SPEED_SCALE = 1.0f; // 速度缩放系数

};
