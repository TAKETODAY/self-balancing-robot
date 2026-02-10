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

#ifndef BLE_LOG_SERVICE_H
#define BLE_LOG_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ble/log.h"

// 日志服务UUID（自定义）
#define BLE_LOG_SERVICE_UUID         0xF0A0
#define BLE_LOG_CHARACTERISTIC_UUID  0xF0A1

// 日志服务句柄
typedef struct ble_log_service_t ble_log_service_t;

// 初始化BLE日志服务
ble_log_service_t* ble_log_service_init(void);

// 启动BLE日志服务
bool ble_log_service_start(ble_log_service_t* service);

// 停止BLE日志服务
void ble_log_service_stop(ble_log_service_t* service);

// 销毁BLE日志服务
void ble_log_service_deinit(ble_log_service_t* service);

// 设置日志配置
bool ble_log_service_set_config(ble_log_service_t* service,
                               const ble_log_config_t* config);

// 获取当前配置
const ble_log_config_t* ble_log_service_get_config(ble_log_service_t* service);

// 获取日志转发器（用于直接操作）
ble_log_forwarder_t* ble_log_service_get_forwarder(ble_log_service_t* service);

// 检查是否有客户端连接
bool ble_log_service_is_connected(ble_log_service_t* service);

#endif // BLE_LOG_SERVICE_H