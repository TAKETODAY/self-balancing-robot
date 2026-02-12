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

#include "esp/misc.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

inline uint64_t micros() {
  return (uint64_t) esp_timer_get_time();
}

inline uint64_t millis() {
  return esp_timer_get_time() / 1000ULL;
}

inline void delay_microseconds(const uint32_t us) {
  esp_rom_delay_us(us);
}

inline void delay(const uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

inline float min(float a, float b) {
  if (a < b) {
    return a;
  }

  return b;
}

inline float max(float a, float b) {
  return a >= b ? a : b;
}
