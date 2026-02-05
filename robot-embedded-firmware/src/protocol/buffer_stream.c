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

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "protocol/buffer_stream.h"


// 默认错误信息（用于buf为NULL的情况）
static const error_info_t DEFAULT_ERROR = {
  .code = BUFFER_ERR_NULL_POINTER,
  .function = "unknown",
  .line = 0,
  .extra_info = 0
};

error_info_t buffer_get_last_error(const buffer_t* buf) {
  if (buf == NULL) {
    return DEFAULT_ERROR;
  }
  return buf->last_error;
}

void buffer_clear_error(buffer_t* buf) {
  if (buf != NULL) {
    buf->last_error.code = BUFFER_OK;
    buf->last_error.function = "";
    buf->last_error.line = 0;
    buf->last_error.extra_info = 0;
  }
}

unsigned int buffer_get_error_count(const buffer_t* buf) {
  return buf ? buf->error_count : 0;
}

bool buffer_has_error(const buffer_t* buf) {
  return buf ? buf->last_error.code != BUFFER_OK : true;
}

const char* buffer_error_to_string(buffer_error_t error) {
  switch (error) {
    case BUFFER_OK: return "成功";
    case BUFFER_ERR_NULL_POINTER: return "空指针错误";
    case BUFFER_ERR_INVALID_ARG: return "无效参数";
    case BUFFER_ERR_OVERFLOW: return "缓冲区溢出";
    case BUFFER_ERR_UNDERFLOW: return "缓冲区下溢";
    case BUFFER_ERR_INSUFFICIENT_SPACE: return "空间不足";
    case BUFFER_ERR_OUT_OF_BOUNDS: return "访问越界";
    case SERIALIZE_ERR_INVALID_TYPE: return "无效的消息类型";
    case SERIALIZE_ERR_INVALID_DATA: return "无效的数据";
    case SERIALIZE_ERR_STRING_TOO_LONG: return "字符串过长";
    case SERIALIZE_ERR_CHECKSUM: return "校验和错误";
    case SERIALIZE_ERR_FORMAT: return "格式错误";
    case SYS_ERR_NO_MEMORY: return "内存不足";
    case SYS_ERR_TIMEOUT: return "超时";
    default: return "未知错误";
  }
}

void buffer_print_error(const buffer_t* buf, const char* prefix) {
  if (buf == NULL || buf->last_error.code == BUFFER_OK) {
    return;
  }

  const error_info_t* err = &buf->last_error;
  const char* err_str = buffer_error_to_string(err->code);

  printf("%s错误: %s (0x%02X)\n", prefix ? prefix : "", err_str, err->code);
  printf("  位置: %s:%d\n", err->function, err->line);

  if (err->extra_info != 0) {
    switch (err->code) {
      case BUFFER_ERR_INSUFFICIENT_SPACE:
        printf("  需要: %zu 字节, 实际可用: %zu 字节\n",
          err->extra_info, buf->capacity - buf->pos);
        break;
      case BUFFER_ERR_OUT_OF_BOUNDS:
        printf("  访问位置: %zu, 缓冲区大小: %zu\n",
          err->extra_info, buf->capacity);
        break;
      case SERIALIZE_ERR_STRING_TOO_LONG:
        printf("  字符串长度: %zu\n", err->extra_info);
        break;
      default:
        printf("  额外信息: %zu\n", err->extra_info);
        break;
    }
  }

  printf("  缓冲区状态: pos=%zu/%zu (%.1f%%)\n",
    buf->pos, buf->capacity,
    (float) buf->pos / buf->capacity * 100);
}

bool buffer_write_u16(buffer_t* buf, uint16_t value) {
  _BUFFER_CHECK_NULL(buf);

  // 检查空间
  if (buf->pos + 2 > buf->capacity) {
    _BUFFER_SET_ERROR(buf, BUFFER_ERR_INSUFFICIENT_SPACE, 2);
    return false;
  }

  // 执行写入
  uint16_t net_value = htons(value);
  memcpy(buf->data + buf->pos, &net_value, 2);
  buf->pos += 2;

  // 清除之前的错误（因为本次操作成功）
  buffer_clear_error(buf);
  return true;
}

bool buffer_read_u16(buffer_t* buf, uint16_t* value) {
  _BUFFER_CHECK_NULL(buf);
  if (value == NULL) {
    _BUFFER_SET_ERROR(buf, BUFFER_ERR_NULL_POINTER, 0);
    return false;
  }

  // 检查是否有足够数据
  if (buf->pos + 2 > buf->capacity) {
    _BUFFER_SET_ERROR(buf, BUFFER_ERR_UNDERFLOW, buf->capacity - buf->pos);
    return false;
  }

  // 执行读取
  uint16_t net_value;
  memcpy(&net_value, buf->data + buf->pos, 2);
  *value = ntohs(net_value);
  buf->pos += 2;

  buffer_clear_error(buf);
  return true;
}

bool buffer_write_string(buffer_t* buf, const char* str) {
  _BUFFER_CHECK_NULL(buf);

  if (str == NULL) {
    _BUFFER_SET_ERROR(buf, BUFFER_ERR_NULL_POINTER, 0);
    return false;
  }

  size_t len = strlen(str);

  // 检查字符串长度（限制为65535）
  if (len > 0xFFFF) {
    _BUFFER_SET_ERROR(buf, SERIALIZE_ERR_STRING_TOO_LONG, len);
    return false;
  }

  // 写入长度
  if (!buffer_write_u16(buf, (uint16_t) len)) {
    // 错误信息已由buffer_write_u16设置
    return false;
  }

  // 写入内容（如果有）
  if (len > 0) {
    if (buf->pos + len > buf->capacity) {
      _BUFFER_SET_ERROR(buf, BUFFER_ERR_INSUFFICIENT_SPACE, len);
      return false;
    }

    memcpy(buf->data + buf->pos, str, len);
    buf->pos += len;
  }

  buffer_clear_error(buf);
  return true;
}

// ==================== 初始化函数（更新） ====================

void buffer_init(buffer_t* buf, uint8_t* storage, size_t capacity) {
  if (buf && storage && capacity > 0) {
    buf->data = storage;
    buf->capacity = capacity;
    buf->pos = 0;
    buffer_clear_error(buf);
    buf->error_count = 0;
  }
}

// 从数组创建缓冲区的宏（初始化错误状态）
#define BUFFER_FROM_ARRAY(arr) { \
    .data = (uint8_t*)(arr), \
    .capacity = sizeof(arr), \
    .pos = 0, \
    .last_error = {BUFFER_OK, "", 0, 0}, \
    .error_count = 0 \
}
