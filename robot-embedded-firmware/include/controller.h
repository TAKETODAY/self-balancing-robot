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

// Callback function type for handling received data
typedef controller_error_t (*data_callback_t)(uint8_t* rx_buffer, uint16_t len);

typedef void (*conn_callback_t)(bool connected);

// Initialize the controller with a callback function for handling received data
controller_error_t controller_init(data_callback_t data_callback, conn_callback_t conn_callback);

// Send data through the controller
controller_error_t controller_send(const uint8_t* buffer, size_t length);

// Check if controller client is currently connected
bool controller_is_connected();

#ifdef __cplusplus
}
#endif
