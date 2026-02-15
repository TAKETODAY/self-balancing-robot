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

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  ROBOT_STATE_STANDING,
  ROBOT_STATE_CROUCHING

} robot_state;

typedef enum {
  // 系统状态
  STATE_INITIALIZING,
  STATE_CALIBRATING,
  STATE_ERROR,

  // 核心平衡状态
  STATE_STANDING,  // 静止站立平衡
  STATE_BALANCING, // 主动移动平衡

  // 动作状态
  STATE_CROUCHING,   // 正在蹲下
  STATE_CROUCHED,    // 蹲下保持
  STATE_STANDING_UP, // 正在起立
  STATE_JUMPING,     // 跳跃中（包含起跳、腾空、落地）
  STATE_WALKING,     // 行走中

  // 安全状态
  STATE_EMERGENCY_STOP,
} RobotState;

// 机器人运动状态枚举
typedef enum {
  FORWARD = 0,
  BACK,
  RIGHT,
  LEFT,
  STOP,
  JUMP,
} QR_State_t;

void robot_init();

void robot_set_height(uint8_t percentage);

void robot_set_speed(uint16_t left_wheel_speed, uint16_t right_wheel_speed);

void robot_set_joy(int8_t x, int8_t y);

void robot_stop();

void robot_recover();

bool robot_controller_is_connected();

#ifdef __cplusplus
}
#endif
