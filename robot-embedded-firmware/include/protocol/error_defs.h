// Copyright 2025 -2026 the original author or authors.
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
#include <stddef.h>

// 错误码枚举（按模块分类）
typedef enum {
  // 成功
  BUFFER_OK = 0,

  // 通用错误 (0x00-0x0F)
  BUFFER_ERR_NULL_POINTER = 0x01,
  BUFFER_ERR_INVALID_ARG = 0x02,

  // 缓冲区操作错误 (0x10-0x1F)
  BUFFER_ERR_OVERFLOW = 0x10, // 缓冲区溢出
  BUFFER_ERR_UNDERFLOW = 0x11, // 缓冲区下溢
  BUFFER_ERR_INSUFFICIENT_SPACE = 0x12, // 空间不足
  BUFFER_ERR_OUT_OF_BOUNDS = 0x13, // 访问越界

  // 序列化/反序列化错误 (0x20-0x2F)
  SERIALIZE_ERR_INVALID_TYPE = 0x20,
  SERIALIZE_ERR_INVALID_DATA = 0x21,
  SERIALIZE_ERR_STRING_TOO_LONG = 0x22,
  SERIALIZE_ERR_CHECKSUM = 0x23,
  SERIALIZE_ERR_FORMAT = 0x24,

  // 系统错误 (0xF0-0xFF)
  SYS_ERR_NO_MEMORY = 0xF0,
  SYS_ERR_TIMEOUT = 0xF1,
} buffer_error_t;

// 错误信息结构（记录详细的错误上下文）
typedef struct {
  buffer_error_t code; // 错误码
  const char* function; // 发生错误的函数名
  int line; // 发生错误的行号
  size_t extra_info; // 额外信息（如需要的大小、位置等）
} error_info_t;

