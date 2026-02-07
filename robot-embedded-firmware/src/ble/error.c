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

#include <string.h>
#include <stdio.h>

#include "ble/error.h"

#if !defined(NDEBUG)

const char* ble_error_to_string(const ble_error_t error) {
  switch (error) {
    case BLE_OK: return "OK";
    case BLE_PENDING: return "Operation pending";
    case BLE_ALREADY_DONE: return "Operation already done";
    case BLE_NOT_CONNECTED: return "Not connected";
    case BLE_CONNECTION_LOST: return "Connection lost";
    case BLE_CONNECTION_BUSY: return "Connection busy";
    case BLE_CONNECTION_TIMEOUT: return "Connection timeout";
    case BLE_DEVICE_NOT_FOUND: return "Device not found";
    case BLE_INVALID_PARAM: return "Invalid parameter";
    case BLE_BUFFER_TOO_SMALL: return "Buffer too small";
    case BLE_DATA_TOO_LONG: return "Data too long";
    case BLE_WRITE_FAILED: return "Write failed";
    case BLE_READ_FAILED: return "Read failed";
    case BLE_NOTIFICATION_NOT_ENABLED: return "Notification not enabled";
    case BLE_INDICATION_NOT_ENABLED: return "Indication not enabled";
    case BLE_BUFFER_ALLOCATE_FAILED: return "Buffer allocation failed";
    case BLE_NOT_INITIALIZED: return "BLE not initialized";
    case BLE_INTERNAL_ERROR: return "Internal error";
    case BLE_PLATFORM_ERROR: return "Platform error";
    case BLE_OUT_OF_MEMORY: return "Out of memory";
    case BLE_UNKNOWN: return "Unknown error";
    default: return "Invalid error code";
  }
}

#else

// Release模式：最小化字符串存储
const char* ble_error_to_string(ble_error_t err) {
  static char error_buf[16];
  switch (err) {
    case BLE_OK: return "OK";
    case BLE_NOT_CONNECTED: return "未连接";
    case BLE_INVALID_PARAM: return "参数错误";
    case BLE_BUFFER_TOO_SMALL: return "缓冲区小";
    case BLE_DATA_TOO_LONG: return "数据太长";
    case BLE_UNKNOWN: return "未知错误";

    default:
      snprintf(error_buf, sizeof(error_buf), "错误0x%02X", err);
      return error_buf;
  }
}

#endif // NDEBUG
