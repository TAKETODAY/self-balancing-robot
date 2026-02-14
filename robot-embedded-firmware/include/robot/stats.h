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

#ifndef STATS_COLLECTOR_H
#define STATS_COLLECTOR_H

#include "defs.h"

#include "protocol/message/status_report.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 统计回调函数类型
 *
 * 当收集器触发时，会调用此函数。模块应将当前状态填充到 out_buffer 中，
 * 并在 out_len 中写入数据长度（不能超过 buffer_size）。
 *
 * @param report 汇报数据
 * @param user_data 注册时传入的用户指针
 */
typedef bool (*stats_callback_t)(status_report_t* report, void* user_data);

typedef void (*status_report_callback_t)(const status_report_t* status_report);

/**
 * @brief 初始化统计收集器
 */
void stats_collector_init();

/**
 * @brief 注册一个统计回调
 *
 * @param cb 回调函数指针
 * @param type status type
 * @param user_data 用户上下文（传入回调）
 * @param interval_ms 上报间隔（毫秒），0表示不自动上报
 * @return int 注册句柄（>=0 表示成功，-1 表示失败）
 */
int stats_register_callback(stats_callback_t cb, status_type_t type, void* user_data, uint32_t interval_ms);

/**
 * @brief 注销回调
 *
 * @param handle 注册时返回的句柄
 */
void stats_unregister_callback(int handle);

/**
 * @brief 手动触发一次所有统计回调的收集
 *
 * 此函数会遍历所有已注册的回调，收集数据后通过通信控制器发送。
 * 如果发送失败，数据会被丢弃。
 */
void stats_collect_all(void);

/**
 * @brief 获取当前已注册的回调数量
 */
uint8_t stats_get_callback_count(void);

void stats_collector_tick(uint32_t current_time_ms, status_report_callback_t status_report_cb);

#ifdef __cplusplus
}
#endif
#endif // STATS_COLLECTOR_H
