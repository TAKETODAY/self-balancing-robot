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

#include "ble/log.h"
#include "esp_log_level.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// 日志条目结构
typedef struct {
  esp_log_level_t level; // 日志级别
  char tag[16];          // 标签（最多15字符 + '\0'）
  char message[192];     // 消息内容
  uint32_t timestamp;    // 时间戳
  const char* file;      // 文件名
  int line;              // 行号
} log_entry_t;

// 日志转发器结构
struct ble_log_forwarder_t {
  ble_log_config_t config;  // 配置
  QueueHandle_t log_queue;  // 日志队列
  TaskHandle_t task_handle; // 任务句柄
  SemaphoreHandle_t mutex;  // 互斥锁

  // 回调函数
  ble_log_ready_callback_t ready_callback;
  ble_log_error_callback_t error_callback;

  // 统计信息
  ble_log_stats_t stats;

  // 状态
  bool is_running;
  bool is_initialized;
  bool ble_connected;

  // 原始日志函数
  vprintf_like_t original_log_func;
};

// 默认配置
static const ble_log_config_t DEFAULT_CONFIG = {
  .include_timestamp = true,
  .include_tag = true,
  .include_level = true,
  .include_color = false,
  .include_file_line = false,
  .max_line_length = 200,
  .filter_level = BLE_LOG_LEVEL_INFO
};

// 级别转换
static const char* level_to_string(esp_log_level_t level) {
  switch (level) {
    case ESP_LOG_ERROR: return "ERROR";
    case ESP_LOG_WARN: return "WARN";
    case ESP_LOG_INFO: return "INFO";
    case ESP_LOG_DEBUG: return "DEBUG";
    case ESP_LOG_VERBOSE: return "VERBOSE";
    default: return "UNKNOWN";
  }
}

// 级别缩写
static const char* level_to_short(esp_log_level_t level) {
  switch (level) {
    case ESP_LOG_ERROR: return "E";
    case ESP_LOG_WARN: return "W";
    case ESP_LOG_INFO: return "I";
    case ESP_LOG_DEBUG: return "D";
    case ESP_LOG_VERBOSE: return "V";
    default: return "U";
  }
}

// 颜色代码
static const char* level_to_color(esp_log_level_t level) {
  switch (level) {
    case ESP_LOG_ERROR: return "\033[1;31m";   // 红色
    case ESP_LOG_WARN: return "\033[1;33m";    // 黄色
    case ESP_LOG_INFO: return "\033[1;32m";    // 绿色
    case ESP_LOG_DEBUG: return "\033[1;36m";   // 青色
    case ESP_LOG_VERBOSE: return "\033[1;35m"; // 紫色
    default: return "\033[0m";                 // 重置
  }
}

// 日志格式化函数
static void format_log_entry(const ble_log_forwarder_t* forwarder,
  const log_entry_t* entry,
  char* buffer, size_t buffer_size) {
  if (!buffer || buffer_size == 0) return;

  char* pos = buffer;
  size_t remaining = buffer_size;
  int written = 0;

  // 颜色开始（如果启用）
  if (forwarder->config.include_color) {
    const char* color = level_to_color(entry->level);
    written = snprintf(pos, remaining, "%s", color);
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 时间戳（如果启用）
  if (forwarder->config.include_timestamp) {
    uint32_t seconds = entry->timestamp / 1000;
    uint32_t milliseconds = entry->timestamp % 1000;
    written = snprintf(pos, remaining, "[%lu.%03lu] ", seconds, milliseconds);
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 级别（如果启用）
  if (forwarder->config.include_level) {
    written = snprintf(pos, remaining, "%s ", level_to_short(entry->level));
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 标签（如果启用）
  if (forwarder->config.include_tag && entry->tag[0] != '\0') {
    written = snprintf(pos, remaining, "(%s) ", entry->tag);
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 文件和行号（如果启用）
  if (forwarder->config.include_file_line && entry->file && entry->line > 0) {
    // 只取文件名，去掉路径
    const char* filename = strrchr(entry->file, '/');
    if (!filename) filename = strrchr(entry->file, '\\');
    if (filename) filename++;
    else filename = entry->file;

    written = snprintf(pos, remaining, "[%s:%d] ", filename, entry->line);
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 消息内容
  if (entry->message[0] != '\0') {
    // 限制长度
    size_t msg_len = strlen(entry->message);
    if (msg_len > remaining - 1) {
      msg_len = remaining - 1;
    }

    memcpy(pos, entry->message, msg_len);
    pos += msg_len;
    remaining -= msg_len;
  }

  // 颜色结束（如果启用）
  if (forwarder->config.include_color) {
    written = snprintf(pos, remaining, "\033[0m");
    if (written > 0) {
      pos += written;
      remaining -= written;
    }
  }

  // 确保以换行结束
  if (remaining >= 2) {
    *pos++ = '\n';
    *pos = '\0';
  }
  else if (remaining == 1) {
    *pos = '\0';
  }
}

// 自定义日志输出函数
static int ble_log_vprintf(const char* fmt, va_list args) {
  // 获取当前日志信息
  esp_log_level_t level = esp_log_default_level;
  const char* tag = "";

  // 从fmt中解析标签和消息（简化版）
  // 注意：实际需要更复杂的解析，这里简化处理
  char buffer[256];
  int len = vsnprintf(buffer, sizeof(buffer), fmt, args);

  if (len <= 0) {
    return len;
  }

  // 查找当前日志任务
  TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
  ble_log_forwarder_t* forwarder = pvTaskGetThreadLocalStoragePointer(current_task, 0);

  if (!forwarder || !forwarder->is_running) {
    // 如果没有转发器，使用原始日志函数
    if (forwarder && forwarder->original_log_func) {
      return forwarder->original_log_func(fmt, args);
    }
    return len;
  }

  // 检查级别过滤
  esp_log_level_t current_level = esp_log_level_get(tag);
  if (level < current_level) {
    // 低于设置级别，不转发
    return len;
  }

  // 转换为我们的过滤级别
  bool should_forward = false;
  switch (forwarder->config.filter_level) {
    case BLE_LOG_LEVEL_ERROR:
      should_forward = (level == ESP_LOG_ERROR);
      break;
    case BLE_LOG_LEVEL_WARN:
      should_forward = (level >= ESP_LOG_WARN);
      break;
    case BLE_LOG_LEVEL_INFO:
      should_forward = (level >= ESP_LOG_INFO);
      break;
    case BLE_LOG_LEVEL_DEBUG:
      should_forward = (level >= ESP_LOG_DEBUG);
      break;
    case BLE_LOG_LEVEL_VERBOSE:
      should_forward = true;
      break;
    default:
      should_forward = false;
      break;
  }

  if (!should_forward) {
    return len;
  }

  // 创建日志条目
  log_entry_t entry = {
    .level = level,
    .timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS,
    .file = "",
    .line = 0
  };

  // 复制标签（简化：标签通常在前面的参数中，这里简化处理）
  strncpy(entry.tag, tag, sizeof(entry.tag) - 1);
  entry.tag[sizeof(entry.tag) - 1] = '\0';

  // 复制消息
  strncpy(entry.message, buffer, sizeof(entry.message) - 1);
  entry.message[sizeof(entry.message) - 1] = '\0';

  // 发送到队列
  if (xQueueSend(forwarder->log_queue, &entry, pdMS_TO_TICKS(10)) != pdTRUE) {
    // 队列满，丢弃日志
    xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
    forwarder->stats.dropped_logs++;
    xSemaphoreGive(forwarder->mutex);
  }

  // 同时使用原始日志函数输出到串口
  if (forwarder->original_log_func) {
    forwarder->original_log_func(fmt, args);
  }

  return len;
}

// 日志处理任务
static void log_processing_task(void* arg) {
  ble_log_forwarder_t* forwarder = arg;

  while (forwarder->is_running) {
    log_entry_t entry;

    // 等待日志条目
    if (xQueueReceive(forwarder->log_queue, &entry, pdMS_TO_TICKS(100)) == pdTRUE) {
      // 格式化日志
      char formatted[256];
      format_log_entry(forwarder, &entry, formatted, sizeof(formatted));

      // 更新统计
      xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
      forwarder->stats.total_logs++;

      switch (entry.level) {
        case ESP_LOG_ERROR: forwarder->stats.errors_sent++;
          break;
        case ESP_LOG_WARN: forwarder->stats.warnings_sent++;
          break;
        case ESP_LOG_INFO: forwarder->stats.infos_sent++;
          break;
        case ESP_LOG_DEBUG: forwarder->stats.debugs_sent++;
          break;
        default: break;
      }

      forwarder->stats.current_queue_size = uxQueueMessagesWaiting(forwarder->log_queue);
      if (forwarder->stats.current_queue_size > forwarder->stats.max_queue_size) {
        forwarder->stats.max_queue_size = forwarder->stats.current_queue_size;
      }
      xSemaphoreGive(forwarder->mutex);

      // 通过回调发送
      if (forwarder->ready_callback) {
        forwarder->ready_callback(formatted);
      }
    }

    // 检查队列大小
    UBaseType_t queue_size = uxQueueMessagesWaiting(forwarder->log_queue);
    if (queue_size > 50) {
      // 队列过大，清理一些
      ESP_LOGW("BLE_LOG", "Log queue too large (%lu), clearing", queue_size);
      xQueueReset(forwarder->log_queue);

      if (forwarder->error_callback) {
        forwarder->error_callback(1, "Log queue overflow");
      }
    }
  }

  vTaskDelete(NULL);
}

// 初始化日志转发器
ble_log_forwarder_t* ble_log_forwarder_init(const ble_log_config_t* config) {
  ble_log_forwarder_t* forwarder = malloc(sizeof(ble_log_forwarder_t));
  if (!forwarder) {
    return NULL;
  }

  memset(forwarder, 0, sizeof(ble_log_forwarder_t));

  // 设置配置
  if (config) {
    memcpy(&forwarder->config, config, sizeof(ble_log_config_t));
  }
  else {
    memcpy(&forwarder->config, &DEFAULT_CONFIG, sizeof(ble_log_config_t));
  }

  // 创建队列
  forwarder->log_queue = xQueueCreate(100, sizeof(log_entry_t));
  if (!forwarder->log_queue) {
    free(forwarder);
    return NULL;
  }

  // 创建互斥锁
  forwarder->mutex = xSemaphoreCreateMutex();
  if (!forwarder->mutex) {
    vQueueDelete(forwarder->log_queue);
    free(forwarder);
    return NULL;
  }

  // 保存原始日志函数
  forwarder->original_log_func = esp_log_set_vprintf(ble_log_vprintf);

  // 初始化统计
  memset(&forwarder->stats, 0, sizeof(ble_log_stats_t));

  forwarder->is_initialized = true;

  return forwarder;
}

// 销毁日志转发器
void ble_log_forwarder_deinit(ble_log_forwarder_t* forwarder) {
  if (!forwarder) return;

  ble_log_forwarder_stop(forwarder);

  if (forwarder->log_queue) {
    vQueueDelete(forwarder->log_queue);
  }

  if (forwarder->mutex) {
    vSemaphoreDelete(forwarder->mutex);
  }

  // 恢复原始日志函数
  if (forwarder->original_log_func) {
    esp_log_set_vprintf(forwarder->original_log_func);
  }

  free(forwarder);
}

// 启动日志转发
bool ble_log_forwarder_start(ble_log_forwarder_t* forwarder) {
  if (!forwarder || !forwarder->is_initialized) {
    return false;
  }

  if (forwarder->is_running) {
    return true; // 已经在运行
  }

  forwarder->is_running = true;

  // 创建处理任务
  if (xTaskCreate(log_processing_task,
        "ble_log_task",
        4096,
        forwarder,
        5,
        &forwarder->task_handle) != pdPASS) {
    forwarder->is_running = false;
    return false;
  }

  return true;
}

// 停止日志转发
void ble_log_forwarder_stop(ble_log_forwarder_t* forwarder) {
  if (!forwarder) return;

  forwarder->is_running = false;

  if (forwarder->task_handle) {
    vTaskDelete(forwarder->task_handle);
    forwarder->task_handle = NULL;
  }
}

// 设置回调函数
void ble_log_forwarder_set_callbacks(
  ble_log_forwarder_t* forwarder,
  ble_log_ready_callback_t ready_cb,
  ble_log_error_callback_t error_cb
) {
  if (!forwarder) return;

  forwarder->ready_callback = ready_cb;
  forwarder->error_callback = error_cb;
}

// 手动发送日志
bool ble_log_forwarder_send(ble_log_forwarder_t* forwarder,
  esp_log_level_t level, const char* tag, const char* format, ...) {
  if (!forwarder || !forwarder->is_running) {
    return false;
  }

  va_list args;
  va_start(args, format);

  log_entry_t entry = {
    .level = level,
    .timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS,
    .file = "",
    .line = 0
  };

  if (tag) {
    strncpy(entry.tag, tag, sizeof(entry.tag) - 1);
    entry.tag[sizeof(entry.tag) - 1] = '\0';
  }

  vsnprintf(entry.message, sizeof(entry.message), format, args);
  va_end(args);

  if (xQueueSend(forwarder->log_queue, &entry, pdMS_TO_TICKS(10)) != pdTRUE) {
    xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
    forwarder->stats.dropped_logs++;
    xSemaphoreGive(forwarder->mutex);
    return false;
  }

  return true;
}

// 获取当前配置
const ble_log_config_t* ble_log_forwarder_get_config(ble_log_forwarder_t* forwarder) {
  if (!forwarder)
    return NULL;
  return &forwarder->config;
}

// 更新配置
bool ble_log_forwarder_update_config(ble_log_forwarder_t* forwarder,
  const ble_log_config_t* config) {
  if (!forwarder || !config) return false;

  xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
  memcpy(&forwarder->config, config, sizeof(ble_log_config_t));
  xSemaphoreGive(forwarder->mutex);

  return true;
}

// 获取统计信息
ble_log_stats_t ble_log_forwarder_get_stats(ble_log_forwarder_t* forwarder) {
  ble_log_stats_t stats = { 0 };

  if (forwarder) {
    xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
    memcpy(&stats, &forwarder->stats, sizeof(ble_log_stats_t));
    xSemaphoreGive(forwarder->mutex);
  }

  return stats;
}

// 清空队列
void ble_log_forwarder_clear_queue(ble_log_forwarder_t* forwarder) {
  if (!forwarder || !forwarder->log_queue) return;

  xQueueReset(forwarder->log_queue);

  xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
  forwarder->stats.current_queue_size = 0;
  xSemaphoreGive(forwarder->mutex);
}

// 检查是否连接
bool ble_log_forwarder_is_connected(ble_log_forwarder_t* forwarder) {
  if (!forwarder) return false;

  xSemaphoreTake(forwarder->mutex, portMAX_DELAY);
  bool connected = forwarder->ble_connected;
  xSemaphoreGive(forwarder->mutex);

  return connected;
}
