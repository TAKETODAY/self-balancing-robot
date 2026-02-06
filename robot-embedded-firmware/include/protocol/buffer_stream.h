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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "error_defs.h"

typedef struct {
  uint8_t* data; // 数据指针
  size_t capacity; // 缓冲区总容量
  size_t pos; // 当前读写位置

  // 错误处理相关字段
  error_info_t last_error; // 最后一次错误信息
  unsigned int error_count; // 错误发生次数（用于统计）
} buffer_t;


/**
 * @brief 获取缓冲区的最后一次错误信息
 * @param buf 缓冲区指针
 * @return 错误信息结构体（如果buf为NULL，返回默认错误）
 */
error_info_t buffer_get_last_error(const buffer_t* buf);

/**
 * @brief 清除缓冲区的错误状态
 * @param buf 缓冲区指针
 */
void buffer_clear_error(buffer_t* buf);

/**
 * @brief 获取缓冲区的错误计数
 * @param buf 缓冲区指针
 * @return 错误发生次数
 */
unsigned int buffer_get_error_count(const buffer_t* buf);

/**
 * @brief 检查缓冲区是否有错误
 * @param buf 缓冲区指针
 * @return true 有错误，false 无错误
 */
bool buffer_has_error(const buffer_t* buf);

/**
 * @brief 将错误码转换为可读的字符串
 * @param error 错误码
 * @return 错误描述字符串
 */
const char* buffer_error_to_string(buffer_error_t error);

/**
 * @brief 打印详细的错误信息（调试用）
 * @param buf 缓冲区指针
 * @param prefix 日志前缀
 */
void buffer_print_error(const buffer_t* buf, const char* prefix);

// ==================== 内部使用的错误设置宏 ====================

// 内部宏：设置错误信息（不暴露给外部用户）
#define _BUFFER_SET_ERROR(buf, err_code, extra) \
    do { \
        if ((buf) != NULL) { \
            (buf)->last_error.code = (err_code); \
            (buf)->last_error.function = __func__; \
            (buf)->last_error.line = __LINE__; \
            (buf)->last_error.extra_info = (size_t)(extra); \
            (buf)->error_count++; \
        } \
    } while(0)

// 安全检查宏（简化错误设置）
#define _BUFFER_CHECK_NULL(buf) \
    do { \
        if ((buf) == NULL) { \
            return false; \
        } \
    } while(0)

#define _BUFFER_CHECK_NULL_RET(buf, ret_val) \
    do { \
        if ((buf) == NULL) { \
            return (ret_val); \
        } \
    } while(0)
