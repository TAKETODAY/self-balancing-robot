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

#include "buffer_def.h"

#ifdef __cplusplus
extern "C" {
#endif

bool buffer_has_error(const buffer_t* buf);

error_info_t buffer_get_last_error(const buffer_t* buf);

buffer_error_t buffer_clear_error(buffer_t* buf);

const char* buffer_error_to_string(buffer_error_t error);

#define BUFFER_PRINT_ERROR(buf, format, ...) \
  do { \
     ESP_LOGE("BUFFER", "ERROR: 0x%02X: %s | %s:%d | " format, \
                (buf).last_error.code, \
                buffer_error_to_string((buf).last_error.code), \
                ((buf).last_error.file) ? (buf).last_error.file : "unknown", \
                (buf).last_error.line, \
                ##__VA_ARGS__); \
  } while(0)

#ifdef __cplusplus
}
#endif
