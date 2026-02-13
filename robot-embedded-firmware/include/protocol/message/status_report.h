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
#include "../buffer.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum : uint8_t {
  status_battery = 1,
  status_robot_height = 2,

} status_type_t;

typedef struct {
  float voltage;
  uint8_t percentage;
} statuc_battery_t;

typedef struct {
  status_type_t type;

  union {
    statuc_battery_t battery;
    percentage_t robot_height;
  };

} status_report_t;

/**
 * @brief 序列化状态报告消息到缓冲区
 *
 * 将指定的状态报告消息序列化并写入到提供的缓冲区中。
 *
 * @param msg 指向要序列化的状态报告结构体的指针
 * @param buf 指向目标缓冲区的指针
 * @return bool 序列化成功返回true，失败返回false
 */
bool status_report_serialize(status_report_t* msg, buffer_t* buf);

/**
 * @brief 从缓冲区反序列化状态报告消息
 *
 * 从提供的缓冲区中读取数据并反序列化为状态报告消息。
 *
 * @param msg 指向存储反序列化结果的状态报告结构体的指针
 * @param buf 指向源缓冲区的指针
 * @return bool 反序列化成功返回true，失败返回false
 */
bool status_report_deserialize(status_report_t* msg, buffer_t* buf);

/**
 * @brief 创建指定类型的状态报告消息
 *
 * 根据给定的状态类型初始化一个新的状态报告结构体。
 *
 * @param type 要创建的状态报告类型
 * @return status_report_t 初始化后的状态报告结构体
 */
status_report_t status_report_create(status_type_t type);

#ifdef __cplusplus
}
#endif
