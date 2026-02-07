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

#ifdef __cplusplus
extern "C" {



#endif

typedef enum : uint8_t {
  // 成功
  BUFFER_OK = 0,

  // 通用错误 (0x00-0x0F)
  BUFFER_NULL_POINTER = 0x01,
  BUFFER_INVALID_ARG = 0x02,

  // 缓冲区操作错误 (0x10-0x1F)
  BUFFER_OVERFLOW = 0x10, // 缓冲区溢出
  BUFFER_UNDERFLOW = 0x11, // 缓冲区下溢
  BUFFER_INSUFFICIENT_SPACE = 0x12, // 空间不足
  BUFFER_OUT_OF_BOUNDS = 0x13, // 访问越界

  BUFFER_INVALID_SIZE = 0x14,

  SYS_ERR_NO_MEMORY = 0xF0,
  SYS_ERR_TIMEOUT = 0xF1,
} buffer_error_t;

typedef struct {
  buffer_error_t code;
  const char* file;
  uint16_t line;
  const char* function;
  const char* message;
} error_info_t;

typedef struct {
  uint8_t* data;
  size_t capacity;

  size_t pos;

  error_info_t last_error;
} buffer_t;

#ifdef __cplusplus
}
#endif
