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


bool robot_message_serialize(robot_message_t* msg, buffer_t* buf) {
  return buffer_write_u16(buf, msg->sequence)
         && buffer_write_u8(buf, msg->type)
         && buffer_write_u8(buf, msg->flags);
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
