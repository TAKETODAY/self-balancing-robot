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

#include "robot/leg.h"

#include "logging.hpp"
#include "robot.hpp"
#include "STSServoDriver.hpp"
#include "freertos/FreeRTOS.h"

static auto TAG = "robot-leg";

STSServoDriver servos;

#define LEFT  1
#define RIGHT 2
#define numberOfServos 2

#define SERVO_LEFT_MIN  2050 // 舵机1最低位置
#define SERVO_RIGHT_MIN  2050 // 舵机2最低位置

// #define SERVO_LEFT_MAX (2047 + 12 + 8.4 * (35 + 10)) // 2438 舵机1最高 2600
// #define SERVO_RIGHT_MAX (2047 - 12 - 8.4 * (35 + 10)) // 1658 舵机2最高 1490

#define SERVO_LEFT_MAX 2600 // 2438 舵机1最高 2600
#define SERVO_RIGHT_MAX 1490 // 1658 舵机2最高 1490

#define SERVO_LEFT_ACC 100 // 舵机1加速度，不能太快，否则影响其他动作平衡
#define SERVO_RIGHT_ACC 100 // 舵机2加速度

#define SERVO_LEFT_SPEED 800 // 舵机1速度，不能太快，否则影响其他动作平衡
#define SERVO_RIGHT_SPEED 800 // 舵机2速度

constexpr static byte ID[2] = { 1, 2 };

static struct {
  byte left_acceleration;
  byte right_acceleration;

  int left_position;
  int right_position;

  uint8_t left_position_percentage;
  uint8_t right_position_percentage;

  int left_speed;
  int right_speed;

} handle = {
  .left_acceleration = SERVO_LEFT_ACC,
  .right_acceleration = SERVO_RIGHT_ACC,
  .left_position = 2300,
  .right_position = 2300,
  .left_position_percentage = 50,
  .right_position_percentage = 50,
  .left_speed = SERVO_LEFT_SPEED,
  .right_speed = SERVO_RIGHT_SPEED
};

void robot_leg_init() {
  log_info("robot legs initializing");

  if (!servos.init(&serial2, 1000000)) {
    log_error("robot legs init failed");
  }
  else {
    log_info("robot legs initialized");
    servos.setMode(LEFT, STSMode::POSITION);
    servos.setMode(RIGHT, STSMode::POSITION);

    robot_leg_set_speed(SERVO_LEFT_SPEED, SERVO_RIGHT_SPEED);
    robot_leg_set_acceleration(SERVO_RIGHT_ACC);
    robot_leg_set_height_percentage(50);
  }
}

void robot_leg_set_acceleration(const uint8_t acceleration) {
  handle.left_acceleration = acceleration;
  handle.right_acceleration = acceleration;
  servos.setTargetAcceleration(LEFT, acceleration);
  servos.setTargetAcceleration(RIGHT, acceleration);
}

void robot_leg_set_speed(const int left_speed, const int right_speed) {
  handle.left_speed = left_speed;
  handle.right_speed = right_speed;

  servos.setTargetVelocity(LEFT, left_speed);
  servos.setTargetVelocity(RIGHT, right_speed);
}

void robot_leg_set_height_percentage(uint8_t percentage) {
  percentage = constrain(percentage, 0, 100);

  const uint16_t left_pos = mapi(percentage, 0, 100, SERVO_LEFT_MIN, SERVO_LEFT_MAX);
  const uint16_t right_pos = mapi(percentage, 0, 100, SERVO_RIGHT_MIN, SERVO_RIGHT_MAX);

  handle.left_position = left_pos;
  handle.right_position = right_pos;

  handle.left_position_percentage = percentage;
  handle.right_position_percentage = percentage;

  const int positions[2] = {
    left_pos,
    right_pos
  };

  const int speeds[2] = {
    handle.left_speed,
    handle.right_speed
  };

  servos.setTargetPositions(2, ID, positions, speeds);
}

void robot_leg_set_left_height_percentage(uint8_t percentage) {
  percentage = constrain(percentage, 0, 100);

  const uint16_t left_pos = mapi(percentage, 0, 100, SERVO_LEFT_MIN, SERVO_LEFT_MAX);
  handle.left_position = left_pos;
  handle.left_position_percentage = percentage;
  servos.setTargetPosition(LEFT, left_pos);
}

void robot_leg_set_right_height_percentage(uint8_t percentage) {
  percentage = constrain(percentage, 0, 100);

  const uint16_t right_pos = mapi(percentage, 0, 100, SERVO_RIGHT_MAX, SERVO_RIGHT_MIN);
  handle.right_position = right_pos;
  handle.right_position_percentage = percentage;
  servos.setTargetPosition(RIGHT, right_pos);
}

uint8_t robot_leg_get_left_height_percentage() {
  return handle.left_position_percentage;
}

uint8_t robot_leg_get_right_height_percentage() {
  return handle.right_position_percentage;
}

uint8_t robot_leg_get_height_percentage() {
  return handle.right_position_percentage;
}
