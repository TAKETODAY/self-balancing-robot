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

// robot_log_system.c

#include "ble/log_service.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble/log.h"

// 机器人日志系统
void robot_log_system_init(void) {
  ESP_LOGI("ROBOT_LOG", "初始化机器人日志系统...");

  // 初始化BLE日志服务
  ble_log_service_t* log_service = ble_log_service_init();
  if (!log_service) {
    ESP_LOGE("ROBOT_LOG", "无法初始化BLE日志服务");
    return;
  }

  // 配置日志格式
  ble_log_config_t config = {
    .include_timestamp = true,
    .include_tag = true,
    .include_level = true,
    .include_color = false,    // BLE不支持ANSI颜色
    .include_file_line = true, // 包含文件行号
    .max_line_length = 180,
    .filter_level = BLE_LOG_LEVEL_INFO // 只转发INFO及以上
  };

  ble_log_service_set_config(log_service, &config);

  // 启动服务
  if (!ble_log_service_start(log_service)) {
    ESP_LOGE("ROBOT_LOG", "无法启动BLE日志服务");
    return;
  }

  ESP_LOGI("ROBOT_LOG", "BLE日志服务已启动");
  ESP_LOGI("ROBOT_LOG", "设备名称: ESP32-Logger");
  ESP_LOGI("ROBOT_LOG", "等待手机连接...");
}

// 手动发送特定类型的日志
void robot_log_critical_error(const char* error_msg, const char* file, int line) {
  ble_log_forwarder_t* forwarder = ble_log_service_get_forwarder(
    ble_log_service_init());

  if (forwarder) {
    char msg[256];
    snprintf(msg, sizeof(msg), "CRITICAL ERROR: %s (at %s:%d)",
      error_msg, file, line);

    ble_log_forwarder_send(forwarder, ESP_LOG_ERROR, "ROBOT", msg);
  }
}

// 机器人特定日志宏
#define ROBOT_LOG_ERROR(tag, format, ...) \
    do { \
        ESP_LOGE(tag, format, ##__VA_ARGS__); \
        if (ble_log_service_is_connected(ble_log_service_init())) { \
            char buf[128]; \
            snprintf(buf, sizeof(buf), format, ##__VA_ARGS__); \
            ble_log_forwarder_send(ble_log_service_get_forwarder( \
                ble_log_service_init()), \
                ESP_LOG_ERROR, tag, buf); \
        } \
    } while(0)

#define ROBOT_LOG_WARN(tag, format, ...) \
    ESP_LOGW(tag, format, ##__VA_ARGS__)

#define ROBOT_LOG_INFO(tag, format, ...) \
    ESP_LOGI(tag, format, ##__VA_ARGS__)

// 日志统计任务
void log_statistics_task(void* arg) {
  (void) arg;

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(30000)); // 每30秒

    ble_log_forwarder_t* forwarder = ble_log_service_get_forwarder(
      ble_log_service_init());

    if (forwarder) {
      ble_log_stats_t stats = ble_log_forwarder_get_stats(forwarder);

      ESP_LOGI("LOG_STATS", "日志统计:");
      ESP_LOGI("LOG_STATS", "  总计: %lu", stats.total_logs);
      ESP_LOGI("LOG_STATS", "  错误: %lu", stats.errors_sent);
      ESP_LOGI("LOG_STATS", "  警告: %lu", stats.warnings_sent);
      ESP_LOGI("LOG_STATS", "  信息: %lu", stats.infos_sent);
      ESP_LOGI("LOG_STATS", "  调试: %lu", stats.debugs_sent);
      ESP_LOGI("LOG_STATS", "  丢弃: %lu", stats.dropped_logs);
      ESP_LOGI("LOG_STATS", "  队列大小: %lu/%lu",
        stats.current_queue_size, stats.max_queue_size);
    }
  }
}

void app_main1() {

  // 初始化机器人日志系统
  robot_log_system_init();

  // 创建日志统计任务
  xTaskCreate(log_statistics_task, "log_stats", 2048, NULL, 1, NULL);

  // 示例日志
  ESP_LOGI("MAIN", "机器人系统启动");
  ESP_LOGE("MAIN", "这是一个错误日志示例");
  ESP_LOGW("MAIN", "这是一个警告日志示例");

  // 手动发送自定义日志
  robot_log_critical_error("电机初始化失败", __FILE__, __LINE__);

  // 使用机器人专用宏
  ROBOT_LOG_ERROR("MOTOR", "左轮电机堵转");
  ROBOT_LOG_INFO("SENSOR", "IMU校准完成");
}
