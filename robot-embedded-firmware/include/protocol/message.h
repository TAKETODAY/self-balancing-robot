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
#include "buffer.h"
#include "message/common.h"
#include "message/status_report.h"

#ifdef __cplusplus
extern "C" {



#endif

#define FLAG_DIRECTION    (1 << 0)  // 位0: 方向
#define FLAG_NEED_ACK     (1 << 1)  // 位1: 需要ACK
#define FLAG_IS_RESPONSE  (1 << 2)  // 位2: 是响应
#define FLAG_RESET_SESS   (1 << 3)  // 位3: 重置会话
#define FLAG_MORE_FRAG    (1 << 4)  // 位4: 更多分片
#define FLAG_COMPRESSED   (1 << 5)  // 位5: 压缩
#define FLAG_URGENT       (1 << 6)  // 位6: 紧急
#define FLAG_RESERVED     (1 << 7)  // 位7: 保留


typedef enum : uint8_t {
  MESSAGE_CONTROL = 1,
  MESSAGE_CONTROL_LEG = 2,
  MESSAGE_CONTROL_HEIGHT = 3,

  MESSAGE_CONFIG_SET = 20,    // 设置参数
  MESSAGE_CONFIG_GET = 21,    // 获取参数
  MESSAGE_FIRMWARE_INFO = 22, // 获取固件信息
  MESSAGE_STATUS_REPORT = 23,

  // 动作与模式类
  MESSAGE_ACTION_PLAY = 40,

  MESSAGE_ACK = 60,
  MESSAGE_ERROR = 61,
  MESSAGE_SENSOR_DATA = 62,

  MESSAGE_EMERGENCY_STOP = 80,
  MESSAGE_EMERGENCY_RECOVER = 81,

} message_type_t;

typedef enum : uint8_t {
  PID = 1,
  PID_PITCH = 2,
  PID_SPEED = 3,

} config_type_t;


// 应答状态码
typedef enum : uint8_t {
  ACK_OK = 0,
  ACK_CRC_ERROR = 1,
  ACK_INVALID_CMD = 2,
  ACK_EXEC_FAILED = 3
} ack_status_t;


typedef struct {
  uint16_t left_wheel_speed;
  uint16_t right_wheel_speed;
} control_message_t;

typedef struct {
  uint8_t left_percentage;
  uint8_t right_percentage;
} control_leg_message_t;


typedef struct {
  float P;
  float I;
  float D;
} config_pid_message_t;

typedef struct {
  config_type_t type;

  union {
    int32_t i32;
    int16_t i16;
    int8_t i8;

    // char str_value[32];

    float f;
    double d;
    config_pid_message_t pid;
  } data;

} config_message_t;

typedef struct {
  config_type_t type;
} config_get_message_t;

typedef struct {
  config_type_t type;

  union {
    config_pid_message_t pid;
    int32_t i32;
    int16_t i16;
    int8_t i8;

    float f;
    double d;
  } data;

} config_response_message_t;

typedef struct {
  config_type_t type;

  union {
    config_pid_message_t pid;

    int32_t i32;
    int16_t i16;
    int8_t i8;

    // char str_value[32];

    float f;
    double d;
  } data;

} config_set_message_t;

typedef struct {
  uint16_t sequence;
  message_type_t type;
  uint8_t flags;

  union {
    control_message_t control;
    control_leg_message_t control_leg;
    percentage_t height;
    config_message_t config;
    config_set_message_t set_config;

    status_report_t status_report;
  };

} robot_message_t;


bool robot_message_serialize(robot_message_t* msg, buffer_t* buf);
bool robot_message_deserialize(robot_message_t* msg, buffer_t* buf);

const char* message_type_to_string(message_type_t type);

robot_message_t robot_message_create(message_type_t type);

#ifdef __cplusplus
}
#endif
