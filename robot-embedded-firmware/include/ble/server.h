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

#include <stdbool.h>
#include "nimble/ble.h"
#include "error.h"

// @formatter:off
#ifdef __cplusplus
extern "C" {
#endif
//@formatter:on

// 16 Bit SPP Service UUID
#define BLE_SVC_SPP_UUID16           0xABF0

// 16 Bit SPP Service Characteristic UUID
#define BLE_SVC_SPP_CHR_UUID16       0xABF1

// Callback function type for handling received BLE data
typedef ble_error_t (*ble_data_callback_t)(uint8_t* rx_buffer, uint16_t len);

// Initialize the BLE server with a callback function for data reception
void ble_server_init(ble_data_callback_t callback, uint16_t mtu);

// Send data over BLE connection
ble_error_t ble_server_send(uint8_t* buffer, size_t length);

// Check if BLE client is currently connected
bool ble_server_is_connected();

#ifdef __cplusplus
}
#endif
