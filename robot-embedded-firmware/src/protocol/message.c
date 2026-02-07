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


#include "protocol/message.h"

#include <stdio.h>

#include "protocol/buffer.h"

static bool serialize_control_message(control_message_t* control, buffer_t* buf) {
  return buffer_write_u16(buf, control->left_wheel_speed)
         && buffer_write_u16(buf, control->right_wheel_speed)
         && buffer_write_u8(buf, control->leg_height_percentage);
}

static bool serialize_body(robot_message_t* msg, buffer_t* buf) {
  switch (msg->type) {
    case MESSAGE_CONTROL:
      return serialize_control_message(&msg->body.control, buf);
    default:
      break;
  }
  return false;
}

bool robot_message_serialize(robot_message_t* msg, buffer_t* buf) {
  return buffer_write_u16(buf, msg->sequence)
         && buffer_write_u8(buf, msg->type)
         && buffer_write_u8(buf, msg->flags)
         && serialize_body(msg, buf);
}

// deserialize

static bool deserialize_control_message(control_message_t* control, buffer_t* buf) {
  return buffer_read_u16(buf, &control->left_wheel_speed)
         && buffer_read_u16(buf, &control->right_wheel_speed)
         && buffer_read_u8(buf, &control->leg_height_percentage);
}

static bool deserialize_config_pid(config_pid_message_t* pid, buffer_t* buf) {
  return buffer_read_f32(buf, &pid->P)
         && buffer_read_f32(buf, &pid->I)
         && buffer_read_f32(buf, &pid->D);
}

static bool deserialize_config_body(config_message_t* config, buffer_t* buf) {
  switch (config->type) {
    case PID_SPEED:
    case PID_PITCH: return deserialize_config_pid(&config->data.pid, buf);

    default:
      break;
  }
  return false;
}


static bool deserialize_config_message(config_message_t* config, buffer_t* buf) {
  return buffer_read_u8(buf, (uint8_t*) &config->type)
         && deserialize_config_body(config, buf);
}

static bool deserialize_body(robot_message_t* msg, buffer_t* buf) {
  switch (msg->type) {
    case MESSAGE_CONTROL: return deserialize_control_message(&msg->body.control, buf);
    case MESSAGE_CONFIG_GET: return deserialize_config_message(&msg->body.config, buf);
    case MESSAGE_CONFIG_SET: return deserialize_config_message(&msg->body.config, buf);
    case MESSAGE_EMERGENCY_STOP: return true; // no body

    default:
      break;
  }
  return false;
}

bool robot_message_deserialize(robot_message_t* msg, buffer_t* buf) {
  return buffer_read_u16(buf, &msg->sequence)
         && buffer_read_u8(buf, (uint8_t*) &msg->type)
         && buffer_read_u8(buf, &msg->flags)
         && deserialize_body(msg, buf);
}

void test() {
  uint8_t data[32];
  buffer_t buf = buffer_create(data, 32);

  robot_message_t msg = {
    .sequence = 100,
    .type = MESSAGE_CONFIG_GET,
    .flags = 0b00001111,
    .body.config = {
      .type = PID_PITCH,
      .data.pid = { 1, 1, 1 }
    }
  };

  if (!robot_message_serialize(&msg, &buf)) {
    error_info_t err = buffer_get_last_error(&buf);

    printf("序列化失败！\n");
    printf("错误码: 0x%02X\n", err.code);
    printf("错误描述: %s\n", buffer_error_to_string(err.code));
    printf("发生位置: %s:%d\n", err.function, err.line);
  }
}

void message_frame(const robot_message_t* message) {
  // 处理紧急消息
  if (message->flags & FLAG_URGENT) {
  }

  // 处理需要应答的消息
  if (message->flags & FLAG_NEED_ACK) {
  }

  // 处理响应消息
  if (message->flags & FLAG_IS_RESPONSE) {
  }
}
