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
#include "servos.hpp"

#include "esp/serial.hpp"
#include "ble/gatt_svc.h"
#include "ble/heart_rate.h"

#include "protocol.h"
#include "ble.h"

// Wrobot wrobot;

static auto TAG = "default";

Wrobot wrobot;

LQRController lqr_controller;

QueueHandle_t buffer_queue = nullptr;

#define RX_QUEUE_LEN 100

static void onDataReceived(uint8_t* data, size_t len);

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


static void robot_control_frame_parsing_task(void* pvParameters) {
  MODLOG_DFLT(INFO, "BLE server started");
  buffer_t buffer;
  for (;;) {
    if (xQueueReceive(buffer_queue, &buffer, portMAX_DELAY)) {
      MODLOG_DFLT(INFO, "收到数据包，长度: %d 字节, 数据地址: %p", buffer.pos, buffer.data);

#ifdef DEBUG_PRINT_DATA
      ESP_LOG_BUFFER_HEX("BLE_RX", buffer.data, buffer.length);
#endif

      free(buffer.data);
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
  servos_init();

  lqr_controller.begin();

  battery_init();

  ble_server_init(onDataReceived);

  buffer_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(buffer_t));
  if (buffer_queue == nullptr) {
    log_error("Failed to create RX queue!");
  }

  xTaskCreate(heart_rate_task, "status_report", 4 * 1024, nullptr, 5, nullptr);
  xTaskCreate(robot_control_frame_parsing_task, "robot_ctrl", 4096, nullptr, 8, nullptr);
}

static void onDataReceived(uint8_t* const data, const size_t len) {
  const buffer_t buffer = {
    .data = data,
    .capacity = len,
    .pos = len,
    .last_error = {},
  };

  if (xQueueSend(buffer_queue, &buffer, 0) != pdTRUE) {
    MODLOG_DFLT(WARN, "BLE RX 队列已满，丢弃数据包");
  }
  else {
    MODLOG_DFLT(DEBUG, "数据包已放入队列等待处理");
  }

}


void on_report_timer_callback(TimerHandle_t xTimer) {
  sensor_data_t sensors;

}
