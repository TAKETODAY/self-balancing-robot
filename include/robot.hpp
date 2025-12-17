// Copyright 2025 the original author or authors.
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

#include "event.hpp"
#include "foc/BLDCMotor.h"

#define SERVO0_MIN  2050 // 舵机1最低位置
#define SERVO1_MIN  2050 // 舵机2最低位置
#define SERVO0_MAX (2047 + 12 + 8.4 * (35 + 10)) // 2438 舵机1最高 2600
#define SERVO1_MAX (2047 - 12 - 8.4 * (35 + 10)) // 1658 舵机2最高 1490
#define SERVO0_ACC 100 // 舵机1加速度，不能太快，否则影响其他动作平衡
#define SERVO1_ACC 100 // 舵机2加速度
#define SERVO0_SPEED 400 // 舵机1速度，不能太快，否则影响其他动作平衡
#define SERVO1_SPEED 400 // 舵机2速度


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
  STATE_STANDING, // 静止站立平衡
  STATE_BALANCING, // 主动移动平衡

  // 动作状态
  STATE_CROUCHING, // 正在蹲下
  STATE_CROUCHED, // 蹲下保持
  STATE_STANDING_UP, // 正在起立
  STATE_JUMPING, // 跳跃中（包含起跳、腾空、落地）
  STATE_WALKING, // 行走中

  // 安全状态
  STATE_EMERGENCY_STOP,
} RobotState;


typedef struct {
  int height = 38;
  int roll;
  int linear;
  int angular;
  int dir;
  int dir_last;
  int joyy;
  int joyy_last;
  int joyx;
  int joyx_last;
  bool go;
  int acc0 = SERVO0_ACC; // 舵机1加速度
  int acc1 = SERVO1_ACC; // 舵机2加速度
  int speed0 = SERVO0_SPEED; // 舵机1速度
  int speed1 = SERVO1_SPEED; // 舵机2速度
} Wrobot;

extern Wrobot wrobot;

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
