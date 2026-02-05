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

#include "protocol/format.h"
#include "protocol/buffer_stream.h"


// 内部辅助函数：序列化控制消息
static bool serialize_control_message(const robot_message_t* msg, buffer_t* buf) {
  if (!buffer_write_i16(buf, msg->body.control.left_wheel_speed)) {
    return false;
  }
  if (!buffer_write_i16(buf, msg->body.control.right_wheel_speed)) {
    return false;
  }
  if (!buffer_write_u8(buf, msg->body.control.leg_height)) {
    return false;
  }
  return true;
}

bool serialize_robot_message(const robot_message_t* msg, buffer_t* buf) {
  if (msg == NULL || buf == NULL) {
    if (buf)
      _BUFFER_SET_ERROR(buf, BUFFER_ERR_NULL_POINTER, 0);
    return false;
  }

  // 保存原始位置，用于回滚
  size_t original_pos = buf->pos;

  // 写入头部
  if (!buffer_write_u16(buf, msg->sequence)) goto rollback;
  if (!buffer_write_u8(buf, (uint8_t) msg->type)) goto rollback;
  if (!buffer_write_u8(buf, msg->flags)) goto rollback;

  // 根据类型序列化载荷
  switch (msg->type) {
    case MESSAGE_CONTROL:
      if (!serialize_control_message(msg, buf)) goto rollback;
      break;

    case MESSAGE_CONFIG_SET:
    case MESSAGE_CONFIG_GET:
      if (!buffer_write_u8(buf, msg->body.config.param_id)) goto rollback;
      if (!buffer_write_u32(buf, msg->body.config.param_value)) goto rollback;
      if (!buffer_write_string(buf, msg->body.config.str_value)) goto rollback;
      break;

    case MESSAGE_STATUS:
      // 无载荷
      break;

    default:
      _BUFFER_SET_ERROR(buf, SERIALIZE_ERR_INVALID_TYPE, msg->type);
      goto rollback;
  }

  // 成功，清除可能的历史错误
  buffer_clear_error(buf);
  return true;

rollback:
  buf->pos = original_pos;
  return false;
}

bool deserialize_robot_message(robot_message_t* msg, buffer_t* buf) {
  if (msg == NULL || buf == NULL) {
    if (buf)
      _BUFFER_SET_ERROR(buf, BUFFER_ERR_NULL_POINTER, 0);
    return false;
  }

  size_t original_pos = buf->pos;

  // 读取头部
  if (!buffer_read_u16(buf, &msg->sequence)) goto rollback;

  uint8_t type_byte;
  if (!buffer_read_u8(buf, &type_byte)) goto rollback;
  msg->type = (message_type_t) type_byte;

  if (!buffer_read_u8(buf, &msg->flags)) goto rollback;

  // 根据类型读取载荷
  switch (msg->type) {
    case MESSAGE_CONTROL:
      if (!buffer_read_i16(buf, &msg->body.control.left_wheel_speed)) goto rollback;
      if (!buffer_read_i16(buf, &msg->body.control.right_wheel_speed)) goto rollback;
      if (!buffer_read_u8(buf, &msg->body.control.leg_height)) goto rollback;
      break;

    case MESSAGE_CONFIG_SET:
    case MESSAGE_CONFIG_GET:
      if (!buffer_read_u8(buf, &msg->body.config.param_id)) goto rollback;
      if (!buffer_read_u32(buf, &msg->body.config.param_value)) goto rollback;

      // 检查字符串缓冲区大小
      if (!buffer_read_string(buf, msg->body.config.str_value,
        sizeof(msg->body.config.str_value))) {
        goto rollback;
      }
      break;

    case MESSAGE_STATUS:
      // 无载荷
      break;

    default:
      _BUFFER_SET_ERROR(buf, SERIALIZE_ERR_INVALID_TYPE, msg->type);
      goto rollback;
  }

  buffer_clear_error(buf);
  return true;

rollback:
  buf->pos = original_pos;
  return false;
}


void process_message_with_error_handling(void) {
  // 1. 创建缓冲区
  uint8_t data[32]; // 故意用较小的缓冲区测试错误
  buffer_t buf = BUFFER_FROM_ARRAY(data);

  // 2. 创建消息
  robot_message_t msg = {
    .sequence = 100,
    .type = MESSAGE_CONFIG_SET,
    .flags = 0x01,
    .body.config = {
      .param_id = 10,
      .param_value = 0x12345678,
      .str_value = "这是一个很长的字符串，可能会超过缓冲区大小"
    }
  };

  // 3. 尝试序列化
  printf("=== 尝试序列化消息 ===\n");
  if (!serialize_robot_message(&msg, &buf)) {
    // 获取详细的错误信息
    error_info_t err = buffer_get_last_error(&buf);

    printf("序列化失败！\n");
    printf("错误码: 0x%02X\n", err.code);
    printf("错误描述: %s\n", buffer_error_to_string(err.code));
    printf("发生位置: %s:%d\n", err.function, err.line);
    printf("错误次数: %u\n", buffer_get_error_count(&buf));

    // 打印详细的错误信息
    buffer_print_error(&buf, "序列化");

    // 根据错误类型采取不同措施
    switch (err.code) {
      case BUFFER_ERR_INSUFFICIENT_SPACE:
        printf("解决方案: 增加缓冲区大小到至少 %zu 字节\n",
          err.extra_info + buffer_size(&buf));
        break;
      case SERIALIZE_ERR_STRING_TOO_LONG:
        printf("解决方案: 缩短字符串长度，当前 %zu，最大 %u\n",
          err.extra_info, 0xFFFF);
        break;
      default:
        printf("请检查代码逻辑\n");
        break;
    }
  }
  else {
    printf("序列化成功！数据大小: %zu 字节\n", buffer_size(&buf));
  }

  printf("\n=== 测试正确的消息 ===\n");

  // 4. 清除错误状态，测试正确的消息
  buffer_clear_error(&buf);
  buffer_rewind(&buf);

  // 创建一个小消息
  robot_message_t small_msg = {
    .sequence = 101,
    .type = MESSAGE_CONTROL,
    .flags = 0x00,
    .body.control = { 100, -80, 50 }
  };

  if (serialize_robot_message(&small_msg, &buf)) {
    printf("小消息序列化成功！\n");
    printf("数据大小: %zu 字节\n", buffer_size(&buf));

    // 反序列化验证
    buffer_rewind(&buf);
    robot_message_t decoded_msg;
    if (deserialize_robot_message(&decoded_msg, &buf)) {
      printf("反序列化成功！\n");
      printf("Sequence: %u\n", decoded_msg.sequence);
    }
    else {
      buffer_print_error(&buf, "反序列化");
    }
  }
}

// 错误统计监控函数
void monitor_buffer_errors(buffer_t* buf, const char* name) {
  static unsigned int last_error_count = 0;

  unsigned int current_count = buffer_get_error_count(buf);
  if (current_count > last_error_count) {
    printf("[%s] 检测到新的错误！总错误数: %u\n",
      name, current_count);

    if (buffer_has_error(buf)) {
      buffer_print_error(buf, name);
    }

    last_error_count = current_count;
  }
}

int main() {
  process_message_with_error_handling();
  return 0;
}
