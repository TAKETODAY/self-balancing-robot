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

#include "robot.hpp"
#include "robot/leg.h"
#include "robot/error.h"

#include "foc/sensors/MagneticSensorI2C.h"
#include "battery.hpp"
#include "logging.hpp"
#include "attitude_sensor.h"
#include "LQRController.hpp"
#include "nvs_flash.h"

#include "esp/serial.hpp"

#include "protocol.h"
#include "ble.h"
#include "controller.h"

static auto TAG = "default";

Wrobot wrobot;

static LQRController lqr_controller;

#define RX_QUEUE_LEN 100

static QueueHandle_t buffer_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(robot_message_t));

static controller_error_t on_data_received(uint8_t* rx_buffer, uint16_t len);
static void conn_state_change(bool connected);

void nvs_init() {
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

static void status_report_task(void* param) {
  log_info("status report task started");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  constexpr TickType_t xFrequency = pdMS_TO_TICKS(2000);

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    robot_message_t message = robot_message_create(MESSAGE_STATUS_REPORT);
    message.status_report = status_report_create(status_robot_height);
    message.status_report.robot_height = { robot_leg_get_height_percentage() };

    byte data[sizeof(robot_message_t)];
    buffer_t buffer = buffer_create(data, sizeof(robot_message_t));

    if (robot_message_serialize(&message, &buffer)) {
      size_t size = buffer_get_size(&buffer);
      if (auto err = controller_send(data, size); err) {
        controller_log_error(err, "send failed");
      }
    }
    else {
      buffer_print_error(buffer, "message serialize failed");
    }
  }
}

static void handle_robot_message(robot_message_t* message) {
  switch (message->type) {
    case MESSAGE_CONTROL: {
      const auto control = message->control;
      robot_set_speed(control.left_wheel_speed, control.right_wheel_speed);
      break;
    }
    case MESSAGE_CONTROL_LEG: {
      const auto control_leg = message->control_leg;
      robot_leg_set_left_height_percentage(control_leg.left_percentage);
      robot_leg_set_right_height_percentage(control_leg.right_percentage);
      break;
    }
    case MESSAGE_CONTROL_HEIGHT: {
      robot_set_height(message->height.percentage);
      break;
    }
    case MESSAGE_EMERGENCY_STOP:
      robot_stop();
      break;
    case MESSAGE_EMERGENCY_RECOVER:
      robot_recover();
      break;
    default:
      break;
  }
}

static void robot_message_parsing_task(void* pvParameters) {
  log_debug("BLE server started");
  for (;;) {
    robot_message_t message;
    if (xQueueReceive(buffer_queue, &message, portMAX_DELAY)) {
      log_info("收到指令，type: %s, sequence: %u, size: %u",
        message_type_to_string(message.type), message.sequence, sizeof(robot_message_t));

      handle_robot_message(&message);
    }
    else {
      log_error("从队列接收数据失败（不应发生）。\n");
    }
  }
}

static controller_error_t on_data_received(uint8_t* rx_buffer, uint16_t len) {
  robot_message_t message{};
  auto buffer = buffer_create(rx_buffer, len);

  if (robot_message_deserialize(&message, &buffer)) {
    if (xQueueSend(buffer_queue, &message, 0) == pdTRUE) {
      log_debug("数据包已放入队列等待处理");
      return CONTROLLER_OK;
    }
    log_warn("Controller RX 队列已满，丢弃数据包");
    return CONTROLLER_CONNECTION_BUSY;
  }

  buffer_print_error(buffer, "Controller message deserialize failed");
  return CONTROLLER_READ_FAILED;
}


static void conn_state_change(bool connected) {
  if (connected) {
    log_info("Controller Connected");
  }
  else {
    log_info("Controller Disconnected");
  }
}

void robot_init() {
  nvs_init();

  serial.begin(115200);
  robot_leg_init();

  lqr_controller.begin();

  battery_init();

  controller_init(on_data_received, conn_state_change);

  xTaskCreate(status_report_task, "status_report", 4096, nullptr, 5, nullptr);
  xTaskCreate(robot_message_parsing_task, "robot_ctrl", 4096, nullptr, 8, nullptr);
}

void robot_stop() {
  lqr_controller.stop();
}

void robot_recover() {
  lqr_controller.recover();
}

bool robot_controller_is_connected() {
  return controller_is_connected();
}

void robot_set_height(const uint8_t percentage) {
  robot_leg_set_height_percentage(percentage);
}

void robot_set_speed(uint16_t left_wheel_speed, uint16_t right_wheel_speed) {

}
