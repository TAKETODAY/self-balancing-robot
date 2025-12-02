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


#include "esp_log.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "battery.hpp"
#include "esp/serial.hpp"
#include "wifi.h"
#include "esp_private/log_level.h"

/**
 * Declare the symbol pointing to the former implementation of esp_restart function
 */
// extern void __real_esp_restart(void);

/**
 * Redefine esp_restart function to print a message before actually restarting
 */
// void __wrap_esp_restart(void) {
//   printf("Restarting in progress...\n");
//   /* Call the former implementation to actually restart the board */
//   __real_esp_restart();
// }


extern "C" void app_main(void) {
  esp_log_set_default_level(ESP_LOG_DEBUG);

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init();
  SerialPort serial;
  serial.begin(1000000, uart_port_t::UART_NUM_1);

  battery_init();
}
