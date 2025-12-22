// Copyright 2025 the original author or authors.
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

#ifndef BLE_GATT_SERVER_H
#define BLE_GATT_SERVER_H

#include <stdbool.h>
#include "esp_gatt_defs.h"

// 定义你的自定义服务UUID（必须唯一）
// 使用在线UUID生成器生成你自己的UUID
#define ROBOT_SERVICE_UUID        0xFF01  // 16位短UUID（实际使用128位完整UUID更佳）
#define ROBOT_RX_CHAR_UUID        0xFF02  // 接收指令特征
#define ROBOT_TX_CHAR_UUID        0xFF03  // 发送状态特征

// 最大传输单元（ATT_MTU），影响单包最大长度，Android通常支持517字节
#define PROTOCOL_MAX_PACKET_SIZE  128

// 初始化BLE GATT服务器
void ble_gatt_server_init(void);

// 通过通知发送数据到安卓App（用于发送传感器状态等）
void ble_send_notification(const uint8_t *data, size_t length);

#endif
