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

#include "robot/stats.h"

#include <string.h>

#include "controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define STATS_MAX_CALLBACKS 4

typedef struct {
  stats_callback_t cb;
  status_type_t status_type;

  void* user_data;
  uint32_t interval_ms;
  uint32_t last_report_ms; // 上次上报时间
  bool in_use;
} stats_entry_t;

static SemaphoreHandle_t mutex = nullptr;
static stats_entry_t entries[STATS_MAX_CALLBACKS];

void stats_collector_init() {
  mutex = xSemaphoreCreateMutex();
  memset(entries, 0, sizeof(entries));
}

int stats_register_callback(const stats_callback_t cb, const status_type_t type, void* user_data, const uint32_t interval_ms) {
  if (!cb) {
    return -1;
  }

  xSemaphoreTake(mutex, portMAX_DELAY);
  int handle = -1;
  for (int i = 0; i < STATS_MAX_CALLBACKS; i++) {
    if (!entries[i].in_use) {
      entries[i].cb = cb;
      entries[i].user_data = user_data;
      entries[i].interval_ms = interval_ms;
      entries[i].last_report_ms = 0;
      entries[i].status_type = type;
      entries[i].in_use = true;
      handle = i;
      break;
    }
  }

  xSemaphoreGive(mutex);
  return handle;
}

void stats_unregister_callback(const int handle) {
  if (handle < 0 || handle >= STATS_MAX_CALLBACKS) {
    return;
  }
  xSemaphoreTake(mutex, portMAX_DELAY);
  entries[handle].in_use = false;
  xSemaphoreGive(mutex);
}

uint8_t stats_get_callback_count(void) {
  uint8_t count = 0;
  xSemaphoreTake(mutex, portMAX_DELAY);
  for (int i = 0; i < STATS_MAX_CALLBACKS; i++) {
    if (entries[i].in_use) {
      count++;
    }
  }
  xSemaphoreGive(mutex);
  return count;
}

void stats_collector_tick(const uint32_t current_time_ms, const status_report_callback_t status_report_cb) {
  if (!mutex || !status_report_cb) {
    return;
  }

  xSemaphoreTake(mutex, portMAX_DELAY);
  for (int i = 0; i < STATS_MAX_CALLBACKS; i++) {
    stats_entry_t* entry = &entries[i];
    if (entry->in_use) {
      const uint32_t elapsed = current_time_ms - entry->last_report_ms;
      if (elapsed >= entry->interval_ms) {
        status_report_t status_report;
        if (entry->cb(&status_report, entry->user_data)) {
          status_report.type = entry->status_type;
          status_report_cb(&status_report);
          entry->last_report_ms = current_time_ms;
        }
      }
    }
  }
  xSemaphoreGive(mutex);
}
