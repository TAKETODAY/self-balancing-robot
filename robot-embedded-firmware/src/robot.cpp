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

#include "robot.hpp"

#include "foc/sensors/MagneticSensorI2C.h"
#include "battery.hpp"
#include "esp_log.h"
#include "AttitudeSensor.hpp"
#include "ble.h"
#include "LQRController.hpp"
#include "nvs_flash.h"
#include "servos.hpp"

#include "esp/serial.hpp"
#include "wifi.h"
#include "ble/gatt_svc.h"
#include "ble/heart_rate.h"

// Wrobot wrobot;

static const char* TAG = "default";

Wrobot wrobot;

LQRController lqr_controller;

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

    serial1.write(val);

    /* Sleep */
    vTaskDelay(HEART_RATE_TASK_PERIOD);
  }

  /* Clean up at exit */
  vTaskDelete(nullptr);
}

void robot_init() {
  nvs_init();

  serial.begin(115200);
  servos_init();

  lqr_controller.begin();

  wifi_init();
  battery_init();

  log_info("ble_init started!");
  ble_init();
  log_info("ble_init end");

  xTaskCreate(heart_rate_task, "Heart Rate", 4 * 1024, nullptr, 5, nullptr);
}
