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

#include "protocol/error.h"

#include <stdio.h>

#include "logging.hpp"

static const char* TAG = "Buffer";

const char* buffer_error_to_string(buffer_error_t error) {
  switch (error) {
    case BUFFER_OK: return "Success";
    case BUFFER_NULL_POINTER: return "Null pointer";
    case BUFFER_INVALID_ARG: return "无效参数";
    case BUFFER_OVERFLOW: return "Buffer overflow";
    case BUFFER_EOF: return "Buffer EOF";
    case BUFFER_INSUFFICIENT_SPACE: return "空间不足";
    case BUFFER_OUT_OF_BOUNDS: return "访问越界";
    case BUFFER_INVALID_SIZE: return "Invalid size";
    default: return "Unknown error";
  }
}

buffer_error_t buffer_clear_error(buffer_t* buf) {
  buf->last_error.code = BUFFER_OK;

  return BUFFER_OK;
}

bool buffer_has_error(const buffer_t* buf) {
  return buf && buf->last_error.code != BUFFER_OK;
}

error_info_t buffer_get_last_error(const buffer_t* buf) {
  if (buf) {
    return buf->last_error;
  }
  return (error_info_t){
    .code = BUFFER_NULL_POINTER,
    .file = __FILE__,
    .function = __func__,
    .line = __LINE__,
    .message = "Buffer pointer is NULL"
  };
}
