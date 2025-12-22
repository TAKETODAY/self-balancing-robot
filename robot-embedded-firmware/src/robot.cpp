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
#include "LQRController.hpp"
#include "nvs_flash.h"
#include "servos.hpp"

#include "esp/serial.hpp"
#include "wifi.h"

// Wrobot wrobot;

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

void robot_init() {
  nvs_init();

  serial.begin(115200);
  servos_init();

  lqr_controller.begin();

  wifi_init();
  battery_init();
}

