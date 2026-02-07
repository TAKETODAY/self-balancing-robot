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

#include "esp_log.h"
#include "robot.hpp"
#include "STSServoDriver.hpp"
#include "esp/gpio.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static auto TAG = "robot-leg";

STSServoDriver servos;

#define LEFT  1
#define RIGHT 2
#define numberOfServos 2

#define SERVO0_MIN  2050 // 舵机1最低位置
#define SERVO1_MIN  2050 // 舵机2最低位置
#define SERVO0_MAX (2047 + 12 + 8.4 * (35 + 10)) // 2438 舵机1最高 2600
#define SERVO1_MAX (2047 - 12 - 8.4 * (35 + 10)) // 1658 舵机2最高 1490
#define SERVO0_ACC 100 // 舵机1加速度，不能太快，否则影响其他动作平衡
#define SERVO1_ACC 100 // 舵机2加速度
#define SERVO0_SPEED 400 // 舵机1速度，不能太快，否则影响其他动作平衡
#define SERVO1_SPEED 400 // 舵机2速度


static struct {
  byte left_acceleration;
  byte right_acceleration;

  int left_position;
  int right_position;

  int left_speed;
  int right_speed;

} handle = {
  .left_acceleration = SERVO0_ACC,
  .right_acceleration = SERVO1_ACC,
  .left_position = 2300,
  .right_position = 2300,
  .left_speed = SERVO0_SPEED,
  .right_speed = SERVO1_SPEED

};

constexpr static byte ID[2] = { 1, 2 };

static int positions[2];
static int speeds[2];

static void servosLoop(void* pvParameters) {
  ESP_LOGI(TAG, "servos looping");

  int offset = SERVO0_MIN;
  int last = 0;
  for (;;) {
    int pos = servos.getCurrentPosition(LEFT) - offset;

    if (last != pos) {
      servos.setTargetPosition(RIGHT, offset - pos);
      // ESP_LOGI(TAG, "Servo1 Position: %d", pos);
      last = pos;
    }
    delay(50);
  }
}


void robot_leg_init() {
  log_info("robot legs initializing");

  if (!servos.init(&serial2, 1000000)) {
    log_error("robot legs init failed");
  }
  else {
    log_info("robot legs initialized");
    servos.setMode(LEFT, STSMode::POSITION);
    servos.setMode(RIGHT, STSMode::POSITION);

    robot_leg_set_speed(SERVO0_SPEED, SERVO1_SPEED);
    robot_leg_set_acceleration(SERVO1_ACC);
    robot_leg_set_height_percentage(50);

    xTaskCreate(servosLoop, "servosLoop", 4096, nullptr, tskIDLE_PRIORITY, nullptr);
  }

  log_info("uart0 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_0));
  log_info("uart1 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_1));
  log_info("uart2 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_2));
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
  speeds[0] = left_speed;
  speeds[1] = right_speed;
  servos.setTargetVelocity(LEFT, left_speed);
  servos.setTargetVelocity(RIGHT, right_speed);
}

void robot_leg_set_height_percentage(uint8_t percentage) {
  if (percentage > 100)
    percentage = 100;

  constexpr static int16_t left_range = SERVO0_MAX - SERVO0_MIN;
  constexpr static int16_t right_range = SERVO1_MAX - SERVO1_MIN;

  const int32_t left_pos = left_range * percentage / 100;
  const int32_t right_pos = right_range * percentage / 100;

  handle.left_position = SERVO0_MAX - left_pos;
  handle.right_position = SERVO1_MAX - right_pos;

  positions[0] = handle.left_position;
  positions[1] = handle.right_position;

  servos.setTargetPositions(2, ID, positions, speeds);
}
