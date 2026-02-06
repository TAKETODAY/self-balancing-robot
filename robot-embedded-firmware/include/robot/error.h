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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 错误码分层结构 ====================
// 高4位：模块ID (0x0-0xF)
// 低4位：具体错误 (0x0-0xF)
// 注意：使用8位错误码，与BLE错误码保持统一格式

// 模块ID定义
typedef enum {
  ROBOT_MODULE_SYSTEM = 0x0, // 系统模块
  ROBOT_MODULE_MOTION = 0x1, // 运动控制
  ROBOT_MODULE_SENSOR = 0x2, // 传感器
  ROBOT_MODULE_CONTROL = 0x3, // 控制算法
  ROBOT_MODULE_CONFIG = 0x4, // 配置管理
  ROBOT_MODULE_POWER = 0x5, // 电源管理
  ROBOT_MODULE_COMM = 0x6, // 通信模块
  ROBOT_MODULE_CALIBRATION = 0x7, // 校准模块
  ROBOT_MODULE_SAFETY = 0x8, // 安全系统
  ROBOT_MODULE_DEBUG = 0x9, // 调试模块
  ROBOT_MODULE_RESERVED_A = 0xA, // 保留A
  ROBOT_MODULE_RESERVED_B = 0xB, // 保留B
  ROBOT_MODULE_RESERVED_C = 0xC, // 保留C
  ROBOT_MODULE_RESERVED_D = 0xD, // 保留D
  ROBOT_MODULE_RESERVED_E = 0xE, // 保留E
  ROBOT_MODULE_UNKNOWN = 0xF // 未知模块
} robot_module_t;

// 详细错误码枚举
typedef enum : uint8_t {
  // ========== 系统模块错误 (0x0X) ==========
  ROBOT_OK = 0x00, // 成功
  ROBOT_SYS_NOT_INIT = 0x01, // 系统未初始化
  ROBOT_SYS_INIT_FAILED = 0x02, // 系统初始化失败
  ROBOT_SYS_BUSY = 0x03, // 系统繁忙
  ROBOT_SYS_TIMEOUT = 0x04, // 系统超时
  ROBOT_SYS_INTERNAL = 0x05, // 内部错误
  ROBOT_SYS_NO_MEMORY = 0x06, // 内存不足
  ROBOT_SYS_HW_FAULT = 0x07, // 硬件故障
  ROBOT_SYS_SW_FAULT = 0x08, // 软件故障

  // ========== 运动控制模块错误 (0x1X) ==========
  ROBOT_MOTION_INVALID_PARAM = 0x10, // 无效运动参数
  ROBOT_MOTION_OUT_OF_RANGE = 0x11, // 运动范围越界
  ROBOT_MOTION_NOT_CALIBRATED = 0x12, // 未校准
  ROBOT_MOTION_SERVO_FAULT = 0x13, // 舵机故障
  ROBOT_MOTION_OVER_CURRENT = 0x14, // 电机过流
  ROBOT_MOTION_OVER_TEMP = 0x15, // 电机过热
  ROBOT_MOTION_STALL_DETECTED = 0x16, // 堵转检测
  ROBOT_MOTION_ENCODER_FAULT = 0x17, // 编码器故障
  ROBOT_MOTION_GEAR_SLIP = 0x18, // 齿轮打滑
  ROBOT_MOTION_POSITION_ERROR = 0x19, // 位置错误

  // ========== 传感器模块错误 (0x2X) ==========
  ROBOT_SENSOR_NOT_FOUND = 0x20, // 传感器未找到
  ROBOT_SENSOR_READ_FAILED = 0x21, // 传感器读取失败
  ROBOT_SENSOR_CALIB_FAILED = 0x22, // 传感器校准失败
  ROBOT_SENSOR_OUT_OF_RANGE = 0x23, // 传感器数据超范围
  ROBOT_SENSOR_NOISE_HIGH = 0x24, // 传感器噪声过高
  ROBOT_SENSOR_IMU_FAULT = 0x25, // IMU故障
  ROBOT_SENSOR_ENCODER_DIRTY = 0x26, // 编码器脏污
  ROBOT_SENSOR_BATTERY_FAULT = 0x27, // 电池传感器故障

  // ========== 控制算法模块错误 (0x3X) ==========
  ROBOT_CTRL_PID_SATURATED = 0x30, // PID饱和
  ROBOT_CTRL_INTEGRAL_WINDUP = 0x31, // 积分饱和
  ROBOT_CTRL_UNSTABLE = 0x32, // 系统不稳定
  ROBOT_CTRL_CONVERGE_FAIL = 0x33, // 收敛失败
  ROBOT_CTRL_GAIN_TOO_HIGH = 0x34, // 增益过高
  ROBOT_CTRL_GAIN_TOO_LOW = 0x35, // 增益过低
  ROBOT_CTRL_FILTER_OVERFLOW = 0x36, // 滤波器溢出

  // ========== 配置管理模块错误 (0x4X) ==========
  ROBOT_CFG_INVALID = 0x40, // 无效配置
  ROBOT_CFG_VERSION_MISMATCH = 0x41, // 配置版本不匹配
  ROBOT_CFG_LOAD_FAILED = 0x42, // 配置加载失败
  ROBOT_CFG_SAVE_FAILED = 0x43, // 配置保存失败
  ROBOT_CFG_CRC_ERROR = 0x44, // 配置CRC错误
  ROBOT_CFG_OUT_OF_BOUNDS = 0x45, // 配置参数越界
  ROBOT_CFG_DEFAULT_USED = 0x46, // 使用默认配置

  // ========== 电源管理模块错误 (0x5X) ==========
  ROBOT_PWR_LOW_BATTERY = 0x50, // 电池电量低
  ROBOT_PWR_CRITICAL_BATTERY = 0x51, // 电池电量严重不足
  ROBOT_PWR_OVER_VOLTAGE = 0x52, // 过压保护
  ROBOT_PWR_UNDER_VOLTAGE = 0x53, // 欠压保护
  ROBOT_PWR_OVER_CURRENT = 0x54, // 过流保护
  ROBOT_PWR_OVER_TEMP = 0x55, // 温度过高
  ROBOT_PWR_CHARGE_FAULT = 0x56, // 充电故障
  ROBOT_PWR_POWER_GOOD_FAIL = 0x57, // 电源监控失效

  // ========== 通信模块错误 (0x6X) ==========
  ROBOT_COMM_BUFFER_OVERFLOW = 0x60, // 缓冲区溢出
  ROBOT_COMM_CHECKSUM_ERROR = 0x61, // 校验和错误
  ROBOT_COMM_TIMEOUT = 0x62, // 通信超时
  ROBOT_COMM_PROTOCOL_ERROR = 0x63, // 协议错误
  ROBOT_COMM_QUEUE_FULL = 0x64, // 队列已满
  ROBOT_COMM_FRAME_ERROR = 0x65, // 帧错误
  ROBOT_COMM_BAUDRATE_ERROR = 0x66, // 波特率错误

  // ========== 校准模块错误 (0x7X) ==========
  ROBOT_CALIB_IN_PROGRESS = 0x70, // 校准进行中
  ROBOT_CALIB_FAILED = 0x71, // 校准失败
  ROBOT_CALIB_ABORTED = 0x72, // 校准被中止
  ROBOT_CALIB_DATA_INVALID = 0x73, // 校准数据无效
  ROBOT_CALIB_NOT_NEEDED = 0x74, // 无需校准
  ROBOT_CALIB_OUT_OF_RANGE = 0x75, // 校准参数越界

  // ========== 安全系统错误 (0x8X) ==========
  ROBOT_SAFE_EMERGENCY_STOP = 0x80, // 急停触发
  ROBOT_SAFE_TILT_DETECTED = 0x81, // 倾斜检测
  ROBOT_SAFE_COLLISION = 0x82, // 碰撞检测
  ROBOT_SAFE_OVERLOAD = 0x83, // 过载检测
  ROBOT_SAFE_TEMPERATURE_HIGH = 0x84, // 温度过高
  ROBOT_SAFE_WATCHDOG_TIMEOUT = 0x85, // 看门狗超时
  ROBOT_SAFE_FALL_DETECTED = 0x86, // 跌倒检测
  ROBOT_SAFE_BOUNDARY_BREACH = 0x87, // 边界突破

  // ========== 调试模块错误 (0x9X) ==========
  ROBOT_DBG_LOG_FULL = 0x90, // 日志已满
  ROBOT_DBG_TRACE_BUFFER_FULL = 0x91, // 跟踪缓冲区满
  ROBOT_DBG_ASSERT_FAILED = 0x92, // 断言失败
  ROBOT_DBG_STACK_OVERFLOW = 0x93, // 栈溢出

  // ========== 未知错误 ==========
  ROBOT_UNKNOWN_ERROR = 0xFF // 未知错误

} robot_error_t;

// ==================== 辅助函数 ====================

/**
 * @brief 从错误码提取模块ID
 */
static inline robot_module_t robot_error_get_module(robot_error_t err) {
  return (robot_module_t) ((err >> 4) & 0x0F);
}

/**
 * @brief 检查错误码是否表示成功
 */
static inline bool robot_error_is_ok(robot_error_t err) {
  return err == ROBOT_OK;
}

/**
 * @brief 检查错误码是否表示失败
 */
static inline bool robot_error_is_error(robot_error_t err) {
  return err != ROBOT_OK;
}

/**
 * @brief 检查是否为安全相关错误（需要紧急处理）
 */
static inline bool robot_error_is_critical(robot_error_t err) {
  robot_module_t module = robot_error_get_module(err);
  return (module == ROBOT_MODULE_SAFETY) ||
         (module == ROBOT_MODULE_POWER && (err & 0x0F) >= 0x5);
}

#ifdef __cplusplus
}
#endif
