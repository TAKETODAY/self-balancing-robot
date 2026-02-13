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


#include "controller/error.h"

const char* controller_error_to_string(const controller_error_t err) {
  switch (err) {
    case CONTROLLER_OK: return "OK";
    case CONTROLLER_NOT_CONNECTED: return "Not connected";
    case CONTROLLER_CONNECTION_LOST: return "Connection lost";
    case CONTROLLER_CONNECTION_BUSY: return "Connection busy";
    case CONTROLLER_CONNECTION_TIMEOUT: return "Connection timeout";
    case CONTROLLER_DEVICE_NOT_FOUND: return "Device not found";
    case CONTROLLER_INVALID_PARAM: return "Invalid parameter";
    case CONTROLLER_BUFFER_TOO_SMALL: return "Buffer too small";
    case CONTROLLER_DATA_TOO_LONG: return "Data too long";
    case CONTROLLER_WRITE_FAILED: return "Write failed";
    case CONTROLLER_READ_FAILED: return "Read failed";
    case CONTROLLER_BUFFER_ALLOCATE_FAILED: return "Buffer allocation failed";
    case CONTROLLER_INTERNAL_ERROR: return "Internal error";
    case CONTROLLER_PLATFORM_ERROR: return "Platform error";
    case CONTROLLER_OUT_OF_MEMORY: return "Out of memory";
    case CONTROLLER_UNKNOWN: return "Unknown error";
    default: return "Invalid error code";
  }
}
