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


#include "servos.hpp"

#include "esp_log.h"
#include "STSServoDriver.hpp"
#include "esp/gpio.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static auto TAG = "servos";

static SerialPort serial2(UART_NUM_2);

STSServoDriver servos;


byte ids[] = { 1, 2 };
int positions[2];
int speeds[] = { 800, 800 };

#define SERVO0_MIN  2050 // 舵机1最低位置
#define SERVO1_MIN  2050 // 舵机2最低位置
#define SERVO0_MAX (2047 + 12 + 8.4 * (35 + 10)) // 2438 舵机1最高 2600
#define SERVO1_MAX (2047 - 12 - 8.4 * (35 + 10)) // 1658 舵机2最高 1490
#define SERVO0_ACC 100 // 舵机1加速度，不能太快，否则影响其他动作平衡
#define SERVO1_ACC 100 // 舵机2加速度
#define SERVO0_SPEED 400 // 舵机1速度，不能太快，否则影响其他动作平衡
#define SERVO1_SPEED 400 // 舵机2速度


static void servosLoop(void* pvParameters);

void servos_init() {
  ESP_LOGI(TAG, "servos initializing");

  if (!servos.init(&serial2, 1000000)) {
    ESP_LOGE(TAG, "servos init failed");
  }
  else {
    ESP_LOGI(TAG, "servos initialized");

    servos.setMode(2, STSMode::POSITION);

    // servos.setTargetAcceleration(1, 100);
    // servos.setTargetAcceleration(2, 100);

    // servos.setTargetPosition(1, 2300);

    // positions[0] = 2300;
    // positions[1] = 2300;
    // servos.setTargetPositions(2, ids, positions, speeds);

    xTaskCreate(servosLoop, "servosLoop", 2048, NULL, tskIDLE_PRIORITY, NULL);
  }

  ESP_LOGI(TAG, "uart0 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_0));
  ESP_LOGI(TAG, "uart1 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_1));
  ESP_LOGI(TAG, "uart2 installed %d", uart_is_driver_installed(uart_port_t::UART_NUM_2));
}

static void servosLoop(void* pvParameters) {
  ESP_LOGI(TAG, "servos looping");

  int offset = SERVO0_MIN;
  int last = 0;
  int last2 = 0;
  for (;;) {
    int pos = servos.getCurrentPosition(1) - offset;

    if (last != pos) {
      servos.setTargetPosition(2, offset - pos);
      // ESP_LOGI(TAG, "Servo1 Position: %d", pos);
      last = pos;
    }
    // pos = offset - servos.getCurrentPosition(2);
    // if (last2 != pos) {
    //   ESP_LOGI(TAG, "Servo2 Position: %d", pos);
    //   last2 = pos;
    // }
    // servos.setTargetPosition(2, pos);
    delay(50);
  }
}
