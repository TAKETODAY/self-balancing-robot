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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "buffer_stream.h"


/**
 * @brief 初始化缓冲区（从已分配内存）
 */
inline void buffer_init(buffer_t* buf, uint8_t* storage, size_t capacity) {
  if (buf && storage && capacity > 0) {
    buf->data = storage;
    buf->capacity = capacity;
    buf->pos = 0;
  }
}

/**
 * @brief 从静态数组创建缓冲区（编译期确定容量）
 */
#define BUFFER_FROM_ARRAY(arr) { .data = (uint8_t*)(arr), .capacity = sizeof(arr), .pos = 0 }

/**
 * @brief 重置读写位置到开头（重用缓冲区）
 */
inline void buffer_rewind(buffer_t* buf) {
  if (buf)
    buf->pos = 0;
}

/**
 * @brief 设置绝对读写位置（用于随机访问）
 */
inline bool buffer_seek(buffer_t* const buf, const size_t position) {
  if (!buf || position > buf->capacity)
    return false;
  buf->pos = position;
  return true;
}

/**
 * @brief 获取当前已写入/读取的数据量
 */
inline size_t buffer_size(const buffer_t* buf) {
  return buf ? buf->pos : 0;
}

/**
 * @brief 获取剩余可用空间（用于写入）
 */
inline size_t buffer_remaining(const buffer_t* buf) {
  return buf ? buf->capacity - buf->pos : 0;
}

/**
 * @brief 检查是否有足够剩余空间
 */
inline bool buffer_has_space(const buffer_t* buf, const size_t required) {
  return buf && buffer_remaining(buf) >= required;
}

/**
 * @brief 检查是否还有数据可读
 */
inline bool buffer_has_data(const buffer_t* buf) {
  return buf && buf->pos < buf->capacity;
}

/**
 * @brief 获取当前读写位置的指针（用于直接操作）
 */
inline uint8_t* buffer_current(buffer_t* buf) {
  return buf ? (buf->data + buf->pos) : NULL;
}

/**
 * @brief 写入原始数据（自动更新pos）
 */
inline bool buffer_write_raw(buffer_t* buf, const void* data, size_t len) {
  if (!buf || !buffer_has_space(buf, len)) return false;
  memcpy(buf->data + buf->pos, data, len);
  buf->pos += len;
  return true;
}

/**
 * @brief 读取原始数据（自动更新pos）
 */
inline bool buffer_read_raw(buffer_t* buf, void* out, size_t len) {
  if (!buf || !out || (buf->pos + len) > buf->capacity)
    return false;
  memcpy(out, buf->data + buf->pos, len);
  buf->pos += len;
  return true;
}

/**
 * @brief 跳过指定字节数
 */
inline bool buffer_skip(buffer_t* buf, size_t len) {
  if (!buf || !buffer_has_space(buf, len)) return false;
  buf->pos += len;
  return true;
}

#include <arpa/inet.h>

// 写入基本类型（网络字节序）
inline bool buffer_write_u16(buffer_t* buf, uint16_t value) {
  uint16_t net_value = htons(value);
  return buffer_write_raw(buf, &net_value, sizeof(net_value));
}

inline bool buffer_write_i16(buffer_t* buf, int16_t value) {
  uint16_t net_value = htons(*(uint16_t*)&value);
  return buffer_write_raw(buf, &net_value, sizeof(net_value));
}

inline bool buffer_write_u32(buffer_t* buf, uint32_t value) {
  uint32_t net_value = htonl(value);
  return buffer_write_raw(buf, &net_value, sizeof(net_value));
}

inline bool buffer_write_u8(buffer_t* buf, uint8_t value) {
  return buffer_write_raw(buf, &value, sizeof(value));
}

inline bool buffer_write_bool(buffer_t* buf, bool value) {
  uint8_t byte_value = value ? 1 : 0;
  return buffer_write_u8(buf, byte_value);
}

// 读取基本类型（网络字节序）
inline bool buffer_read_u16(buffer_t* buf, uint16_t* value) {
  uint16_t net_value;
  if (!buffer_read_raw(buf, &net_value, sizeof(net_value)))
    return false;
  *value = ntohs(net_value);
  return true;
}

inline bool buffer_read_i16(buffer_t* buf, int16_t* value) {
  uint16_t net_value;
  if (!buffer_read_raw(buf, &net_value, sizeof(net_value))) return false;
  *value = (int16_t) ntohs(net_value);
  return true;
}

inline bool buffer_read_u32(buffer_t* buf, uint32_t* value) {
  uint32_t net_value;
  if (!buffer_read_raw(buf, &net_value, sizeof(net_value))) return false;
  *value = ntohl(net_value);
  return true;
}

inline bool buffer_read_u8(buffer_t* buf, uint8_t* value) {
  return buffer_read_raw(buf, value, sizeof(*value));
}

inline bool buffer_read_bool(buffer_t* buf, bool* value) {
  uint8_t byte_value;
  if (!buffer_read_u8(buf, &byte_value)) return false;
  *value = (byte_value != 0);
  return true;
}

/**
 * @brief 写入字符串（格式：2字节长度 + 内容）
 */
inline bool buffer_write_string(buffer_t* buf, const char* str) {
  if (!str) return false;

  size_t len = strlen(str);
  if (len > 0xFFFF) return false; // 字符串过长

  // 先写入长度
  if (!buffer_write_u16(buf, (uint16_t) len)) return false;

  // 再写入内容（如果长度>0）
  if (len > 0) {
    return buffer_write_raw(buf, str, len);
  }

  return true;
}

/**
 * @brief 读取字符串（格式：2字节长度 + 内容）
 */
inline bool buffer_read_string(buffer_t* buf, char* out, size_t max_out_len) {
  if (!out || max_out_len == 0) return false;

  // 先读取长度
  uint16_t len;
  if (!buffer_read_u16(buf, &len)) return false;

  // 检查长度是否合理
  if (len >= max_out_len) return false; // 目标缓冲区太小

  // 读取内容
  if (len > 0) {
    if (!buffer_read_raw(buf, out, len)) return false;
  }

  // 添加终止符
  out[len] = '\0';
  return true;
}

// ==================== 调试与诊断 ====================

/**
 * @brief 获取缓冲区使用率（0.0到1.0）
 */
inline float buffer_usage_ratio(const buffer_t* buf) {
  return buf ? ((float) buf->pos / buf->capacity) : 0.0f;
}

/**
 * @brief 以十六进制打印缓冲区内容（调试用）
 */
inline void buffer_hex_dump(const buffer_t* buf, const char* label) {
  if (!buf || !label) return;

  printf("[%s] 容量=%zu, 位置=%zu (%.1f%%)\n",
    label, buf->capacity, buf->pos, buffer_usage_ratio(buf) * 100);

  for (size_t i = 0; i < buf->pos; i++) {
    if (i % 16 == 0) printf("  %04zX: ", i);
    printf("%02X ", buf->data[i]);
    if (i % 16 == 15 || i == buf->pos - 1) printf("\n");
  }
}
