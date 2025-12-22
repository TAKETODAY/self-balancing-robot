// Copyright 2025 the original author or authors.
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

#include "protocol/parser.h"
#include <string.h>
#include "esp_log.h"

static const char* TAG = "Protocol";

#define SYNC_HEADER_0 0xAA
#define SYNC_HEADER_1 0x55
#define FOOTER_0      0x0D
#define FOOTER_1      0x0A

void protocol_parser_init(protocol_parser_t* parser) {
  memset(parser, 0, sizeof(protocol_parser_t));
}

void protocol_parser_feed(protocol_parser_t* parser, const uint8_t* data, size_t len) {
  // 简单的环形缓冲区写入
  for (size_t i = 0; i < len; i++) {
    parser->buffer[parser->write_pos] = data[i];
    parser->write_pos = (parser->write_pos + 1) % sizeof(parser->buffer);
    if (parser->bytes_available < sizeof(parser->buffer)) {
      parser->bytes_available++;
    }
    else {
      // 缓冲区满，丢弃最老的数据（读位置前进）
      parser->read_pos = (parser->read_pos + 1) % sizeof(parser->buffer);
    }
  }
}

static uint8_t calculate_checksum(const basic_control_frame_t* frame) {
  uint8_t sum = 0;
  const uint8_t* data = (const uint8_t*) &frame->frame_type;
  for (int i = 0; i < frame->data_length; i++) {
    sum += data[i];
  }
  return sum;
}

bool protocol_parser_try_parse(protocol_parser_t* parser, basic_control_frame_t* frame_out) {
  // 需要至少一帧的数据才能开始解析
  if (parser->bytes_available < sizeof(basic_control_frame_t)) {
    return false;
  }

  // 在环形缓冲区中查找同步头
  uint16_t start_search = parser->read_pos;
  for (int i = 0; i < parser->bytes_available - 1; i++) {
    uint16_t pos = (start_search + i) % sizeof(parser->buffer);
    uint16_t next_pos = (pos + 1) % sizeof(parser->buffer);

    // 找到同步头
    if (parser->buffer[pos] == SYNC_HEADER_0 &&
        parser->buffer[next_pos] == SYNC_HEADER_1) {

      // 检查是否有足够数据承载一完整帧
      if (parser->bytes_available - i < sizeof(basic_control_frame_t)) {
        return false; // 数据不够，等待下次接收
      }

      // 从环形缓冲区复制出连续内存以便处理
      uint8_t temp_frame[sizeof(basic_control_frame_t)];
      for (int j = 0; j < sizeof(basic_control_frame_t); j++) {
        temp_frame[j] = parser->buffer[(pos + j) % sizeof(parser->buffer)];
      }

      basic_control_frame_t* frame = (basic_control_frame_t*) temp_frame;

      // 验证帧尾
      if (frame->footer[0] != FOOTER_0 || frame->footer[1] != FOOTER_1) {
        ESP_LOGW(TAG, "无效帧尾");
        parser->read_pos = (pos + 1) % sizeof(parser->buffer); // 跳过这个错误同步头
        parser->bytes_available -= 1;
        continue; // 继续寻找下一个同步头
      }

      // 验证校验和
      if (calculate_checksum(frame) != frame->checksum) {
        ESP_LOGW(TAG, "校验和错误");
        parser->read_pos = (pos + 1) % sizeof(parser->buffer);
        parser->bytes_available -= 1;
        continue;
      }

      // 验证通过，复制到输出结构体
      memcpy(frame_out, frame, sizeof(basic_control_frame_t));

      // 更新缓冲区状态，消费掉这帧数据
      uint16_t consumed = i + sizeof(basic_control_frame_t);
      parser->read_pos = (parser->read_pos + consumed) % sizeof(parser->buffer);
      parser->bytes_available -= consumed;

      ESP_LOGI(TAG, "解析成功: L=%d, R=%d, H=%d, Flags=0x%02X",
        frame->left_wheel_speed, frame->right_wheel_speed,
        frame->leg_height, frame->action_flags);
      return true;
    }
  }
  return false;
}
