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


// #include "esp_log.h"
// #include "nvs_flash.h"

#include "nvs_flash.h"
#include "sdkconfig.h"

#include "robot.hpp"

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
  robot_init();
}
