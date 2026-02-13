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
#include "controller/error.h"

// @formatter:off
#ifdef __cplusplus
extern "C" {
#endif
//@formatter:on

typedef enum {
  CONTROLLER_OK = 0,

  // ========== 连接相关错误 (0x10-0x2F) ==========
  CONTROLLER_NOT_CONNECTED = 0x10,      // 未连接
  CONTROLLER_CONNECTION_LOST = 0x11,    // 连接丢失
  CONTROLLER_CONNECTION_BUSY = 0x12,    // 连接繁忙
  CONTROLLER_CONNECTION_TIMEOUT = 0x13, // 连接超时
  CONTROLLER_DEVICE_NOT_FOUND = 0x14,   // 设备未找到

  // ========== 数据操作错误 (0x30-0x4F) ==========
  CONTROLLER_INVALID_PARAM = 0x30,          // 无效参数
  CONTROLLER_BUFFER_TOO_SMALL = 0x31,       // 缓冲区太小
  CONTROLLER_DATA_TOO_LONG = 0x32,          // 数据太长
  CONTROLLER_WRITE_FAILED = 0x33,           // 写入失败
  CONTROLLER_READ_FAILED = 0x34,            // 读取失败
  CONTROLLER_BUFFER_ALLOCATE_FAILED = 0x37, // 缓冲区分配失败

  CONTROLLER_INTERNAL_ERROR = 0xF1, // 内部错误
  CONTROLLER_PLATFORM_ERROR = 0xF2, // 平台错误
  CONTROLLER_OUT_OF_MEMORY = 0xF3,  // 内存不足
  CONTROLLER_UNKNOWN = 0xFF,        // 未知错误

} controller_error_t;

const char* controller_error_to_string(controller_error_t err);


#if !defined(NDEBUG)
#define controller_log_error(err, format, ...) \
  do { \
      ESP_LOGE("controller", "ERROR: 0x%02X: %s | %s:%d | " format, \
              err, controller_error_to_string(err), __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#else
#define controller_log_error(err, format, ...) \
        ESP_LOGE("controller", "错误 0x%02X: %s", err, controller_error_to_string(err))
#endif


#ifdef __cplusplus
}
#endif
