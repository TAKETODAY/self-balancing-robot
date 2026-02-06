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


#include "ble.h"
#include <string.h>

#if !defined(NDEBUG)

static const char* ble_error_strings[] = {
  // 成功/一般状态
  [BLE_OK] = "操作成功",

  [BLE_PENDING] = "操作进行中",
  [BLE_ALREADY_DONE] = "操作已完成",

  // 连接相关错误
  [BLE_NOT_CONNECTED] = "设备未连接",
  [BLE_CONNECTION_LOST] = "蓝牙连接已丢失",
  [BLE_CONNECTION_BUSY] = "连接繁忙，请稍后重试",
  [BLE_CONNECTION_TIMEOUT] = "连接操作超时",
  [BLE_DEVICE_NOT_FOUND] = "未找到指定的蓝牙设备",

  // 数据操作错误
  [BLE_INVALID_PARAM] = "无效的参数或参数格式错误",
  [BLE_BUFFER_TOO_SMALL] = "提供的缓冲区太小，无法容纳数据",
  [BLE_DATA_TOO_LONG] = "数据长度超过最大限制",
  [BLE_WRITE_FAILED] = "向设备写入数据失败",
  [BLE_READ_FAILED] = "从设备读取数据失败",
  [BLE_NOTIFICATION_NOT_ENABLED] = "通知功能未启用",
  [BLE_INDICATION_NOT_ENABLED] = "指示功能未启用",
  [BLE_BUFFER_ALLOCATE_FAILED] = "缓冲区分配失败",

  // 系统/未知错误
  [BLE_NOT_INITIALIZED] = "BLE子系统未初始化",
  [BLE_INTERNAL_ERROR] = "BLE内部错误",
  [BLE_PLATFORM_ERROR] = "平台相关错误",
  [BLE_OUT_OF_MEMORY] = "内存不足",
  [BLE_UNKNOWN] = "未知错误"
};

const char* ble_error_to_string(ble_error_t err) {
  if (err < 0x10) {
    return ble_error_strings[err];
  }
  else if (err >= 0x10 && err < 0x30) {
    return (err - 0x10 < sizeof(ble_error_strings) / sizeof(ble_error_strings[0])) ? ble_error_strings[err] : "连接相关错误";
  }
  else if (err >= 0x30 && err < 0x50) {
    return (err < sizeof(ble_error_strings) / sizeof(ble_error_strings[0])) ? ble_error_strings[err] : "数据操作错误";
  }
  else if (err >= 0x50 && err < 0x70) {
    return (err < sizeof(ble_error_strings) / sizeof(ble_error_strings[0])) ? ble_error_strings[err] : "配置错误";
  }
  else if (err >= 0xF0) {
    return (err < sizeof(ble_error_strings) / sizeof(ble_error_strings[0])) ? ble_error_strings[err] : "系统错误";
  }

  // 未定义的错误码
  static char unknown_buf[32];
  snprintf(unknown_buf, sizeof(unknown_buf), "未定义错误 (0x%02X)", err);
  return unknown_buf;
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
