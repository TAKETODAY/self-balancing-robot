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

#pragma once

#include "defs.h"

// @formatter:off
#ifdef __cplusplus
extern "C" {
#endif
//@formatter:on

typedef enum : uint8_t {
  BLE_OK = 0,
  BLE_PENDING = 0x01, // 操作进行中
  BLE_ALREADY_DONE = 0x02, // 操作已完成

  // ========== 连接相关错误 (0x10-0x2F) ==========
  BLE_NOT_CONNECTED = 0x10, // 未连接
  BLE_CONNECTION_LOST = 0x11, // 连接丢失
  BLE_CONNECTION_BUSY = 0x12, // 连接繁忙
  BLE_CONNECTION_TIMEOUT = 0x13, // 连接超时
  BLE_DEVICE_NOT_FOUND = 0x14, // 设备未找到


  // ========== 数据操作错误 (0x30-0x4F) ==========
  BLE_INVALID_PARAM = 0x30, // 无效参数
  BLE_BUFFER_TOO_SMALL = 0x31, // 缓冲区太小
  BLE_DATA_TOO_LONG = 0x32, // 数据太长
  BLE_WRITE_FAILED = 0x33, // 写入失败
  BLE_READ_FAILED = 0x34, // 读取失败
  BLE_NOTIFICATION_NOT_ENABLED = 0x35, // 通知未启用
  BLE_INDICATION_NOT_ENABLED = 0x36, // 指示未启用
  BLE_BUFFER_ALLOCATE_FAILED = 0x37, // 缓冲区分配失败
  BLE_QUEUE_FULL = 0x38,


  BLE_NOT_INITIALIZED = 0xF0, // BLE未初始化
  BLE_INTERNAL_ERROR = 0xF1, // 内部错误
  BLE_PLATFORM_ERROR = 0xF2, // 平台错误
  BLE_OUT_OF_MEMORY = 0xF3, // 内存不足
  BLE_UNKNOWN = 0xFF, // 未知错误

} ble_error_t;

const char* ble_error_to_string(ble_error_t err);

/**
 * @brief 记录错误详情（Debug模式记录更多信息）
 *
 * 这个宏用于记录错误发生的位置和上下文
 */
#if !defined(NDEBUG)
#define BLE_LOG_ERROR(err, format, ...) \
        do { \
            ESP_LOGE("BLE", "错误 0x%02X: %s", err, ble_error_to_string(err)); \
            ESP_LOGE("BLE", "位置: %s:%d", __FILE__, __LINE__); \
            if (format[0] != '\0') { \
                ESP_LOGE("BLE", "详情: " format, ##__VA_ARGS__); \
            } \
        } while(0)
#else
// Release模式：只记录错误码和简短消息
#define BLE_LOG_ERROR(err, format, ...) \
        ESP_LOGE("BLE", "错误 0x%02X: %s", err, ble_error_to_string(err))
#endif

/**
 * @brief 检查错误并记录（链式操作常用）
 */
#define BLE_CHECK_AND_LOG(result, fallback) \
    do { \
        ble_error_t __err = (result); \
        if (__err != BLE_OK) { \
            BLE_LOG_ERROR(__err, ""); \
            return (fallback); \
        } \
    } while(0)


#ifdef __cplusplus
}
#endif
