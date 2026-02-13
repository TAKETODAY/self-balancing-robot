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
} Wrobot;

// typedef struct {
//   int joyx; // 左右转向（-100~100），正面看，正数：向左转，负数，向右转
//   int joyy; // 前后移动（-100~100）
//   int jump; // 跳跃标记（0=不跳，1=跳）
//   float roll_adjust; // 左右平衡调整（-5~5），基数0，正面看，正数：向右摆动；负数：向左摆动
//   float leg_height_base; // 腿部基准高度（20~60，核心高度控制参数）,基数20，变小升高，变大降低
//   unsigned long stepDuration; // 步骤持续时间（毫秒）
//   unsigned long pauseDuration; // 步骤执行后的停顿时间（毫秒）
//   // 舵机参数：无需默认值，动作集定义时按需填写，不填则用结构体默认值（或省略）
//   int acc0; // 舵机1加速度 0-250
//   int acc1; // 舵机2加速度 0-250
//   int speed0; // 舵机1速度 0-500
//   int speed1; // 舵机2速度 0-500
// } ActionStep;

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

void robot_set_height(uint8_t percentage);

void robot_set_speed(uint16_t left_wheel_speed, uint16_t right_wheel_speed);

void robot_stop();

void robot_recover();

bool robot_controller_is_connected();

#ifdef __cplusplus
}
#endif
