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
#include "robot/error.h"

#include "foc/sensors/MagneticSensorI2C.h"
#include "battery.hpp"
#include "esp_log.h"
#include "AttitudeSensor.hpp"
#include "LQRController.hpp"
#include "nvs_flash.h"

#include "esp/serial.hpp"
#include "ble/gatt_svc.h"
#include "ble/heart_rate.h"

#include "protocol.h"
#include "ble.h"

static auto TAG = "default";

Wrobot wrobot;

LQRController lqr_controller;

#define MAX_BLE_PACKET_SIZE    512
#define RX_QUEUE_LEN 100

static QueueHandle_t buffer_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(robot_message_t));

static ble_error_t onDataReceived(uint8_t* rx_buffer, uint16_t len);

void nvs_init() {
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void heart_rate_task(void* param) {
  /* Task entry log */
  log_info("heart rate task has been started!");

  /* Loop forever */
  while (true) {
    /* Update heart rate value every 1 second */
    update_heart_rate();
    uint8_t val = get_heart_rate();
    log_info("heart rate updated to %d", val);

    if (auto err = ble_server_send(&val, 1); err) {
      BLE_LOG_ERROR(err, "ble send failed");
    }

    /* Sleep */
    vTaskDelay(HEART_RATE_TASK_PERIOD);
  }

  /* Clean up at exit */
  vTaskDelete(nullptr);
}

static void handle_robot_message(robot_message_t* message) {
  switch (message->type) {
    case MESSAGE_CONTROL: {
      const auto control = message->body.control;
      robot_set_leg_height(control.leg_height_percentage);
      break;
    }
    default:
      break;
  }
}

static void robot_message_parsing_task(void* pvParameters) {
  MODLOG_DFLT(INFO, "BLE server started");
  for (;;) {
    robot_message_t message;
    if (xQueueReceive(buffer_queue, &message, portMAX_DELAY)) {
      MODLOG_DFLT(INFO, "收到指令，type: %u, sequence: %u", message.type, message.sequence);

      handle_robot_message(&message);
    }
    else {
      // 正常情况下，portMAX_DELAY 会导致任务阻塞，不会走到这里。
      MODLOG_DFLT(ERROR, "从队列接收数据失败（不应发生）。\n");
    }
  }
  vTaskDelete(nullptr);
}

void robot_init() {

  nvs_init();

  serial.begin(115200);
  robot_leg_init();

  lqr_controller.begin();

  battery_init();

  ble_server_init(onDataReceived, MAX_BLE_PACKET_SIZE);

  xTaskCreate(heart_rate_task, "status_report", 4096, nullptr, 5, nullptr);
  xTaskCreate(robot_message_parsing_task, "robot_ctrl", 4096, nullptr, 8, nullptr);
}

static ble_error_t onDataReceived(uint8_t* rx_buffer, uint16_t len) {
  log_info("onDataReceived: %u", len);
  robot_message_t message{};
  auto buffer = buffer_create(rx_buffer, len);

  if (robot_message_deserialize(&message, &buffer)) {
    if (xQueueSend(buffer_queue, &message, 0) == pdTRUE) {
      MODLOG_DFLT(DEBUG, "数据包已放入队列等待处理");
      return BLE_OK;
    }
    MODLOG_DFLT(WARN, "BLE RX 队列已满，丢弃数据包");
  }

  buffer_print_error(&buffer, "BLE message serialize failed");
  return BLE_READ_FAILED;
}

void on_report_timer_callback(TimerHandle_t xTimer) {
  sensor_data_t sensors;

}

void robot_set_leg_height(const uint8_t percentage) {
  robot_leg_set_height_percentage(percentage);
}
