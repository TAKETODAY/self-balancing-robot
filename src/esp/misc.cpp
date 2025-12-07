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

#include "esp/platform.hpp"
#include "esp/misc.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

unsigned long micros() {
  return static_cast<unsigned long>(esp_timer_get_time());
}

unsigned long millis() {
  return static_cast<unsigned long>(esp_timer_get_time() / 1000ULL);
}

void delayMicroseconds(uint32_t us) {
  uint64_t m = static_cast<uint64_t>(esp_timer_get_time());
  if (us) {
    uint64_t e = (m + us);
    if (m > e) {
      // overflow
      while (static_cast<uint64_t>(esp_timer_get_time()) > e) {
        NOP();
      }
    }
    while (static_cast<uint64_t>(esp_timer_get_time()) < e) {
      NOP();
    }
  }
}

void delay(const uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

float min(float a, float b) {
  if (a < b) {
    return a;
  }

  return b;
}

float max(float a, float b) {
  return a >= b ? a : b;
}
