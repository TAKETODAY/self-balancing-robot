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

const char* buffer_error_to_string(buffer_error_t error) {
  switch (error) {
    case BUFFER_OK: return "Success";
    case BUFFER_NULL_POINTER: return "Null pointer";
    case BUFFER_INVALID_ARG: return "无效参数";
    case BUFFER_OVERFLOW: return "Buffer overflow";
    case BUFFER_UNDERFLOW: return "Buffer underflow";
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

void buffer_print_error(const buffer_t* buf, const char* prefix) {
  if (!buf) {
    printf("[%s] Cannot print error: Buffer pointer is NULL\n",
      prefix ? prefix : "Buffer");
    return;
  }

  error_info_t error = buf->last_error;

  // 如果没有错误，打印提示信息
  if (error.code == BUFFER_OK) {
    printf("[%s] No error\n", prefix ? prefix : "Buffer");
    return;
  }

  // 打印错误信息
  if (prefix) {
    printf("[%s] ", prefix);
  }

  printf("Error: %s (code: %d)\n",
    buffer_error_to_string(error.code),
    error.code);

  // 打印位置信息（如果可用）
  if (error.file || error.function || error.line > 0) {
    printf("  Location: ");

    if (error.file) {
      printf("%s", error.file);

      if (error.line > 0) {
        printf(":%d", error.line);
      }

      if (error.function) {
        printf(" in %s()", error.function);
      }
    }
    else if (error.function) {
      printf("function %s()", error.function);
    }

    printf("\n");
  }

  // 打印额外的错误消息（如果可用）
  if (error.message && error.message[0] != '\0') {
    printf("  Message: %s\n", error.message);
  }

  // 打印缓冲区状态信息
  printf("  Buffer State: pos=%zu, capacity=%zu, used=%.1f%%\n",
    buf->pos,
    buf->capacity,
    buf->capacity > 0 ? (100.0 * buf->pos / buf->capacity) : 0.0);

  // 如果可能，打印数据片段（用于调试）
  if (buf->data && buf->pos > 0 && buf->pos <= buf->capacity) {
    size_t bytes_to_show = buf->pos < 16 ? buf->pos : 16;
    printf("  Data (first %zu bytes): ", bytes_to_show);

    for (size_t i = 0; i < bytes_to_show; i++) {
      printf("%02X ", buf->data[i]);
    }

    if (buf->pos > bytes_to_show) {
      printf("... (%zu more bytes)", buf->pos - bytes_to_show);
    }
    printf("\n");
  }
}
