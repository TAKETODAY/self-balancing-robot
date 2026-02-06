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


#ifndef ROBOT_ERROR_STRING_H
#define ROBOT_ERROR_STRING_H

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 将机器人错误码转换为可读字符串
 *
 * Debug模式: 返回详细描述
 * Release模式: 返回简短描述或错误码
 *
 * @param err 错误码
 * @return 错误描述字符串（静态常量，无需释放）
 */
const char* robot_error_to_string(robot_error_t err);

/**
 * @brief 获取错误的建议处理措施
 * @param err 错误码
 * @return 处理建议字符串（Debug模式更详细）
 */
const char* robot_error_get_suggestion(robot_error_t err);

/**
 * @brief 获取错误的严重级别
 * @param err 错误码
 * @return 0:信息, 1:警告, 2:错误, 3:严重错误, 4:致命错误
 */
uint8_t robot_error_get_severity(robot_error_t err);

/**
 * @brief 判断错误是否可恢复
 */
bool robot_error_is_recoverable(robot_error_t err);

// ==================== 日志宏 ====================

// 智能错误日志宏（根据NDEBUG调整详细程度）
#if !defined(NDEBUG)
#define ROBOT_LOG_ERROR(err, format, ...) \
        do { \
            if (robot_error_is_error(err)) { \
                ESP_LOGE("ROBOT", "错误 0x%02X [%s]: %s", \
                        err, robot_error_to_string(err), format, ##__VA_ARGS__); \
                ESP_LOGE("ROBOT", "位置: %s:%d, 函数: %s", \
                        __FILE__, __LINE__, __func__); \
                if (robot_error_is_critical(err)) { \
                    ESP_LOGE("ROBOT", "严重错误！需要立即处理！"); \
                } \
            } \
        } while(0)
#else
// Release模式：简化日志
#define ROBOT_LOG_ERROR(err, format, ...) \
        do { \
            if (robot_error_is_error(err)) { \
                if (robot_error_is_critical(err)) { \
                    ESP_LOGE("ROBOT", "严重错误 0x%02X", err); \
                } else { \
                    ESP_LOGW("ROBOT", "错误 0x%02X", err); \
                } \
            } \
        } while(0)
#endif

// 安全检查宏
#define ROBOT_CHECK_ERROR(result) \
    do { \
        robot_error_t __err = (result); \
        if (robot_error_is_error(__err)) { \
            ROBOT_LOG_ERROR(__err, "操作失败"); \
            return __err; \
        } \
    } while(0)

#define ROBOT_CHECK_ERROR_VOID(result) \
    do { \
        robot_error_t __err = (result); \
        if (robot_error_is_error(__err)) { \
            ROBOT_LOG_ERROR(__err, "操作失败"); \
            return; \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // ROBOT_ERROR_STRING_H
