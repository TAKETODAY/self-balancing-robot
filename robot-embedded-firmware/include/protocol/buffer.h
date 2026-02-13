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

#include "error.h"
#include "buffer_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define always_inline __attribute__((always_inline)) inline
#else
#define always_inline inline
#endif

buffer_t* buffer_create_ptr(size_t capacity);

buffer_t buffer_create(uint8_t* memory, size_t capacity);

bool buffer_init(buffer_t* buf, uint8_t* memory, size_t capacity);
bool buffer_reset(buffer_t* buf);

size_t buffer_available(const buffer_t* buf);
bool buffer_is_empty(const buffer_t* buf);
bool buffer_is_full(const buffer_t* buf);

bool buffer_write_raw(buffer_t* buf, const void* data, size_t size);
bool buffer_read_raw(buffer_t* buf, void* data, size_t size);


// 写入操作（网络字节序）
bool buffer_write_u8(buffer_t* buf, uint8_t value);
bool buffer_write_u16(buffer_t* buf, uint16_t value);
bool buffer_write_u32(buffer_t* buf, uint32_t value);
bool buffer_write_i8(buffer_t* buf, int8_t value);
bool buffer_write_i16(buffer_t* buf, int16_t value);
bool buffer_write_i32(buffer_t* buf, int32_t value);
bool buffer_write_f32(buffer_t* buf, float value);
bool buffer_write_bool(buffer_t* buf, bool value);

// 读取操作（从网络字节序转换）
bool buffer_read_u8(buffer_t* buf, uint8_t* value);
bool buffer_read_u16(buffer_t* buf, uint16_t* value);
bool buffer_read_u32(buffer_t* buf, uint32_t* value);
bool buffer_read_i8(buffer_t* buf, int8_t* value);
bool buffer_read_i16(buffer_t* buf, int16_t* value);
bool buffer_read_i32(buffer_t* buf, int32_t* value);
bool buffer_read_f32(buffer_t* buf, float* value);
bool buffer_read_bool(buffer_t* buf, bool* value);

bool buffer_write_string(buffer_t* buf, const char* str);
bool buffer_read_string(buffer_t* buf, char* str);

bool buffer_skip(buffer_t* buf, size_t size);

#ifdef __cplusplus
}
#endif
