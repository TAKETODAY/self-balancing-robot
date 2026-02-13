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

#include <string.h>
#include <arpa/inet.h>

#include "protocol/buffer.h"
#include "protocol/error.h"

#if BUFFER_DEBUG
#define buffer_assert(condition, buf, error) \
    do { \
        if (!(condition)) { \
            if (buf) { \
              buf->last_error.code = error; \
              buf->last_error.file = __FILE__; \
              buf->last_error.function = __func__; \
              buf->last_error.line = __LINE__; \
              buf->last_error.message = ""; \
            } \
            return false; \
        } \
    } while(0)
#else
#define buffer_assert(condition, buf, error) \
    do { \
        if (!(condition)) { \
            if (buf) { \
              buf->last_error.code = error; \
            } \
            return false; \
        } \
    } while(0)
#endif


buffer_t* buffer_create_ptr(const size_t capacity) {
  buffer_t* buffer = malloc(sizeof(buffer_t));
  if (!buffer) {
    return nullptr;
  }

  buffer->data = malloc(capacity);
  if (!buffer->data) {
    free(buffer);
    return nullptr;
  }

  buffer->capacity = capacity;
  buffer->pos = 0;
  memset(&buffer->last_error, 0, sizeof(error_info_t));
  return buffer;
}

buffer_t buffer_create(uint8_t* memory, const size_t capacity) {
  const buffer_t buf = {
    .data = memory,
    .capacity = capacity,
    .pos = 0,
    .last_error = { .code = BUFFER_OK }
  };
  return buf;
}

bool buffer_init(buffer_t* buf, uint8_t* memory, const size_t capacity) {
  buffer_assert(buf && memory, buf, BUFFER_NULL_POINTER);

  buf->data = memory;
  buf->capacity = capacity;
  buf->pos = 0;

  buf->last_error.code = BUFFER_OK;
  return true;
}

buffer_error_t buffer_deinit(buffer_t* buf) {
  if (buf) {
    buf->data = nullptr;
    buf->capacity = 0;
    buf->pos = 0;
  }
  return BUFFER_OK;
}

bool buffer_reset(buffer_t* buf) {
  if (buf) {
    buf->pos = 0;
  }
  return buffer_clear_error(buf);
}


size_t buffer_available(const buffer_t* buf) {
  if (!buf || !buf->data)
    return 0;
  return buf->capacity - buf->pos;
}

inline bool buffer_is_empty(const buffer_t* buf) {
  return buffer_available(buf) == buf->capacity;
}

inline bool buffer_is_full(const buffer_t* buf) {
  return buffer_available(buf) == 0;
}

bool buffer_write_raw(buffer_t* buf, const void* data, const size_t size) {
  buffer_assert(buf && buf->data, buf, BUFFER_NULL_POINTER);
  buffer_assert(data || size == 0, buf, BUFFER_NULL_POINTER);
  buffer_assert(buf->pos + size <= buf->capacity, buf, BUFFER_EOF);

  memcpy(&buf->data[buf->pos], data, size);
  buf->pos += size;
  return true;
}

bool buffer_read_raw(buffer_t* buf, void* data, const size_t size) {
  buffer_assert(buf && buf->data, buf, BUFFER_NULL_POINTER);
  buffer_assert(data && size > 0, buf, BUFFER_NULL_POINTER);
  buffer_assert(buf->pos + size <= buf->capacity, buf, BUFFER_EOF);

  memcpy(data, &buf->data[buf->pos], size);
  buf->pos += size;
  return true;
}

bool buffer_write_u8(buffer_t* buf, const uint8_t value) {
  buffer_assert(buf && buf->data, buf, BUFFER_NULL_POINTER);
  buffer_assert(buf->pos + 1 <= buf->capacity, buf, BUFFER_EOF);

  buf->data[buf->pos++] = value;
  return true;
}

inline bool buffer_write_i8(buffer_t* buf, const int8_t value) {
  return buffer_write_u8(buf, (uint8_t) value);
}

inline bool buffer_write_u16(buffer_t* buf, const uint16_t value) {
  const uint16_t net_value = htons(value);
  return buffer_write_raw(buf, &net_value, 2);
}

inline bool buffer_write_i16(buffer_t* buf, const int16_t value) {
  return buffer_write_u16(buf, (uint16_t) value);
}

inline bool buffer_write_u32(buffer_t* buf, const uint32_t value) {
  const uint32_t net_value = htonl(value);
  return buffer_write_raw(buf, &net_value, 4);
}

inline bool buffer_write_i32(buffer_t* buf, const int32_t value) {
  return buffer_write_u32(buf, (uint32_t) value);
}

inline bool buffer_write_f32(buffer_t* buf, const float value) {
  union {
    float f;
    uint32_t u;
  } converter;

  converter.f = value;
  return buffer_write_u32(buf, converter.u);
}

inline bool buffer_write_bool(buffer_t* buf, const bool value) {
  return buffer_write_u8(buf, value ? 1 : 0);
}

inline bool buffer_read_u8(buffer_t* buf, uint8_t* value) {
  buffer_assert(buf && buf->data, buf, BUFFER_NULL_POINTER);
  buffer_assert(buf->pos + 1 <= buf->capacity, buf, BUFFER_EOF);

  *value = buf->data[buf->pos++];
  return true;
}

inline bool buffer_read_i8(buffer_t* buf, int8_t* value) {
  uint8_t temp;
  const bool ok = buffer_read_u8(buf, &temp);
  if (ok) {
    *value = (int8_t) temp;
  }
  return ok;
}

inline bool buffer_read_u16(buffer_t* buf, uint16_t* value) {
  uint16_t net_value;
  const bool ok = buffer_read_raw(buf, &net_value, 2);
  if (ok) {
    *value = ntohs(net_value);
  }
  return ok;
}

inline bool buffer_read_i16(buffer_t* buf, int16_t* value) {
  uint16_t temp;
  const bool ok = buffer_read_u16(buf, &temp);
  if (ok) {
    *value = (int16_t) temp;
  }
  return ok;
}

inline bool buffer_read_u32(buffer_t* buf, uint32_t* value) {
  uint32_t net_value;
  if (buffer_read_raw(buf, &net_value, 4)) {
    *value = ntohl(net_value);
    return true;
  }
  return false;
}

bool buffer_read_i32(buffer_t* buf, int32_t* value) {
  uint32_t temp;
  const bool ok = buffer_read_u32(buf, &temp);
  if (ok) {
    *value = (int32_t) temp;
  }
  return ok;
}

bool buffer_read_f32(buffer_t* buf, float* value) {
  uint32_t temp;
  const bool ok = buffer_read_u32(buf, &temp);
  if (ok) {
    union {
      float f;
      uint32_t u;
    } converter;

    converter.u = temp;
    *value = converter.f;
  }
  return ok;
}

bool buffer_read_bool(buffer_t* buf, bool* value) {
  uint8_t temp;
  const bool ok = buffer_read_u8(buf, &temp);
  if (ok) {
    *value = temp != 0;
  }
  return ok;
}

bool buffer_write_string(buffer_t* buf, const char* str) {
  const uint16_t len = strlen(str);
  return buffer_write_u16(buf, len)
         && buffer_write_raw(buf, str, len);
}

bool buffer_read_string(buffer_t* buf, char* str) {
  uint16_t len;
  if (buffer_read_u16(buf, &len)
      && buffer_read_raw(buf, str, len)) {
    str[len] = '\0';
    return true;
  }
  return false;
}

bool buffer_skip(buffer_t* buf, size_t size) {
  buffer_assert(buf && buf->data, buf, BUFFER_NULL_POINTER);
  buffer_assert(buf->pos + size <= buf->capacity, buf, BUFFER_EOF);

  buf->pos += size;
  return true;
}

// -------------------------------------------------------------
// ERROR
// -------------------------------------------------------------

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
#if BUFFER_DEBUG
    .file = __FILE__,
    .function = __func__,
    .line = __LINE__,
    .message = "Buffer pointer is NULL"
#endif
  };
}
