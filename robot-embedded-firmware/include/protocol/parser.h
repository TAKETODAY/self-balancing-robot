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

//
// Created by TODAY on 2025/12/21.
//

#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <stdint.h>
#include <stdbool.h>

// 根据你之前的设计定义基础控制帧结构（12字节）
#pragma pack(push, 1)

typedef struct {
  uint8_t sync_header[2]; // 0xAA, 0x55
  uint8_t data_length; // 固定为后续数据长度
  uint8_t frame_type; // 0x01=基础控制帧
  int16_t left_wheel_speed; // 左轮速度
  int16_t right_wheel_speed; // 右轮速度
  uint8_t leg_height; // 腿部高度
  uint8_t action_flags; // 动作标志位
  uint8_t checksum; // 校验和
  uint8_t footer[2]; // 0x0D, 0x0A
} basic_control_frame_t;
#pragma pack(pop)

// 解析器状态
typedef struct {
  uint8_t buffer[512]; // 接收环形缓冲区
  uint16_t write_pos; // 写入位置
  uint16_t read_pos; // 读取/解析位置
  uint16_t bytes_available; // 有效数据长度
} protocol_parser_t;

// 初始化解析器
void protocol_parser_init(protocol_parser_t* parser);

// 向解析器喂入原始蓝牙数据（处理黏包的核心）
void protocol_parser_feed(protocol_parser_t* parser, const uint8_t* data, size_t len);

// 尝试从缓冲区解析出一个完整帧，返回解析到的帧类型
// 解析成功返回true，并通过frame_out返回帧数据

bool protocol_parser_try_parse(protocol_parser_t* parser, basic_control_frame_t* frame_out);


#endif
