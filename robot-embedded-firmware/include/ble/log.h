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

#ifndef BLE_LOG_FORWARDER_H
#define BLE_LOG_FORWARDER_H

#include <stdbool.h>
#include <stdarg.h>
#include "esp_log.h"

// 日志级别过滤
typedef enum {
  BLE_LOG_LEVEL_NONE = 0, // 无日志
  BLE_LOG_LEVEL_ERROR,    // 只转发错误
  BLE_LOG_LEVEL_WARN,     // 警告及以上
  BLE_LOG_LEVEL_INFO,     // 信息及以上
  BLE_LOG_LEVEL_DEBUG,    // 调试及以上
  BLE_LOG_LEVEL_VERBOSE,  // 所有日志
} ble_log_level_t;

// 日志格式选项
typedef struct {
  bool include_timestamp;       // 包含时间戳
  bool include_tag;             // 包含标签
  bool include_level;           // 包含级别
  bool include_color;           // 包含颜色代码
  bool include_file_line;       // 包含文件和行号
  uint16_t max_line_length;     // 最大行长度
  ble_log_level_t filter_level; // 过滤级别
} ble_log_config_t;

// 日志转发器句柄
typedef struct ble_log_forwarder_t ble_log_forwarder_t;

// 回调函数类型
typedef void (*ble_log_ready_callback_t)(const char* log_line);
typedef void (*ble_log_error_callback_t)(int error_code, const char* error_msg);

// 初始化日志转发器
ble_log_forwarder_t* ble_log_forwarder_init(const ble_log_config_t* config);

// 销毁日志转发器
void ble_log_forwarder_deinit(ble_log_forwarder_t* forwarder);

// 启动日志转发
bool ble_log_forwarder_start(ble_log_forwarder_t* forwarder);

// 停止日志转发
void ble_log_forwarder_stop(ble_log_forwarder_t* forwarder);

// 设置回调函数
void ble_log_forwarder_set_callbacks(
  ble_log_forwarder_t* forwarder,
  ble_log_ready_callback_t ready_cb,
  ble_log_error_callback_t error_cb
);

// 手动发送日志（不经过ESP_LOG）
bool ble_log_forwarder_send(ble_log_forwarder_t* forwarder,
  esp_log_level_t level, const char* tag, const char* format, ...);

// 获取当前配置
const ble_log_config_t* ble_log_forwarder_get_config(ble_log_forwarder_t* forwarder);

// 更新配置
bool ble_log_forwarder_update_config(ble_log_forwarder_t* forwarder,
  const ble_log_config_t* config);

// 获取统计信息
typedef struct {
  uint32_t total_logs;
  uint32_t errors_sent;
  uint32_t warnings_sent;
  uint32_t infos_sent;
  uint32_t debugs_sent;
  uint32_t dropped_logs;
  uint32_t max_queue_size;
  uint32_t current_queue_size;
} ble_log_stats_t;

ble_log_stats_t ble_log_forwarder_get_stats(ble_log_forwarder_t* forwarder);

// 清空队列
void ble_log_forwarder_clear_queue(ble_log_forwarder_t* forwarder);

// 检查是否连接
bool ble_log_forwarder_is_connected(ble_log_forwarder_t* forwarder);

#endif // BLE_LOG_FORWARDER_H
