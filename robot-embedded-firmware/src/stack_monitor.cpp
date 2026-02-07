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

#include "stack_monitor.h"
#include <esp_log.h>
#include <vector>

static const char* TAG = "StackMonitor";

static std::vector<task_stack_info_t> monitored_tasks;
static TaskHandle_t monitor_task_handle = nullptr;

static void stack_monitor_task(void* pvParameters) {
  ESP_LOGI(TAG, "Stack monitor task started");

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(5000));

    for (auto& info: monitored_tasks) {
      if (info.task_handle) {
        UBaseType_t current_free = uxTaskGetStackHighWaterMark(info.task_handle);

        // 更新最小空闲值
        if (current_free < info.min_stack_free) {
          info.min_stack_free = current_free;
        }

        // 如果栈使用率超过90%，报警
        float usage_percent = 100.0f * (info.initial_stack - current_free) / info.initial_stack;

        if (usage_percent > 90.0f) {
          ESP_LOGW(TAG, "Task '%s' stack usage: %.1f%% (free: %d words)",
            info.task_name, usage_percent, current_free);
        }
      }
    }

    // 可选：定期打印所有任务栈使用情况
    static int print_counter = 0;
    if (print_counter++ % 6 == 0) {
      // 每30秒打印一次
      print_stack_usage();
    }
  }
}

void init_stack_monitor() {
  xTaskCreate(stack_monitor_task, "StackMonitor", 2048, nullptr, 1, &monitor_task_handle);
}

void register_task_for_monitoring(TaskHandle_t task, const char* name) {
  if (!task || !name) return;

  task_stack_info_t info = {
    .task_handle = task,
    .task_name = name,
    .min_stack_free = portMAX_DELAY, // 初始化为最大值
    .initial_stack = uxTaskGetStackHighWaterMark(task)
  };

  monitored_tasks.push_back(info);
  ESP_LOGI(TAG, "Registered task '%s' for stack monitoring", name);
}

void print_stack_usage() {
  ESP_LOGI(TAG, "=== Stack Usage Report ===");

  for (const auto& info: monitored_tasks) {
    if (info.task_handle) {
      UBaseType_t current_free = uxTaskGetStackHighWaterMark(info.task_handle);
      float usage_percent = 100.0f * (info.initial_stack - current_free) / info.initial_stack;

      ESP_LOGI(TAG, "  %-20s: %5.1f%% used, Free: %4d words, Min free: %4d words",
        info.task_name, usage_percent, current_free, info.min_stack_free);
    }
  }

  // 也可以打印系统所有任务
  ESP_LOGI(TAG, "--- All Tasks ---");
  char task_list_buffer[2048];
  vTaskList(task_list_buffer);
  ESP_LOGI(TAG, "\n%s", task_list_buffer);
}
