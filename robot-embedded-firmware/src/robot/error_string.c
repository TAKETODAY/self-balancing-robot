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

#include "robot/error.h"
#include "robot/error_string.h"

#include <stdio.h>
#include <string.h>

// ==================== Debug模式的详细描述 ====================
#if !defined(NDEBUG)

// 详细错误描述表
static const char* robot_error_strings[] = {
  // 系统模块
  [ROBOT_OK] = "操作成功",
  [ROBOT_SYS_NOT_INIT] = "机器人系统未初始化",
  [ROBOT_SYS_INIT_FAILED] = "系统初始化失败，请检查硬件连接",
  [ROBOT_SYS_BUSY] = "系统繁忙，请稍后重试",
  [ROBOT_SYS_TIMEOUT] = "系统操作超时",
  [ROBOT_SYS_INTERNAL] = "系统内部错误",
  [ROBOT_SYS_NO_MEMORY] = "系统内存不足",
  [ROBOT_SYS_HW_FAULT] = "硬件故障，请检查电路",
  [ROBOT_SYS_SW_FAULT] = "软件故障，请重启系统",

  // 运动控制模块
  [ROBOT_MOTION_INVALID_PARAM] = "无效的运动参数",
  [ROBOT_MOTION_OUT_OF_RANGE] = "运动范围越界，检查限位设置",
  [ROBOT_MOTION_NOT_CALIBRATED] = "运动系统未校准，请先执行校准",
  [ROBOT_MOTION_SERVO_FAULT] = "舵机故障，检查电源和信号线",
  [ROBOT_MOTION_OVER_CURRENT] = "电机电流过大，可能卡住或负载过重",
  [ROBOT_MOTION_OVER_TEMP] = "电机温度过高，请冷却后再使用",
  [ROBOT_MOTION_STALL_DETECTED] = "电机堵转，检查机械结构",
  [ROBOT_MOTION_ENCODER_FAULT] = "编码器故障，无法读取位置",
  [ROBOT_MOTION_GEAR_SLIP] = "齿轮打滑，需要重新调整",
  [ROBOT_MOTION_POSITION_ERROR] = "位置误差过大，检查机械间隙",

  // 安全系统错误
  [ROBOT_SAFE_EMERGENCY_STOP] = "急停按钮被触发",
  [ROBOT_SAFE_TILT_DETECTED] = "机器人倾斜角度过大",
  [ROBOT_SAFE_COLLISION] = "碰撞传感器触发",
  [ROBOT_SAFE_OVERLOAD] = "关节负载过重",
  [ROBOT_SAFE_TEMPERATURE_HIGH] = "关键部件温度过高",
  [ROBOT_SAFE_WATCHDOG_TIMEOUT] = "看门狗超时，系统自动重启",
  [ROBOT_SAFE_FALL_DETECTED] = "跌倒检测触发",
  [ROBOT_SAFE_BOUNDARY_BREACH] = "工作边界被突破",

  // 其他错误...
  [ROBOT_UNKNOWN_ERROR] = "未知错误，请联系技术支持"
};

// 错误处理建议
static const char* robot_error_suggestions[] = {
  [ROBOT_OK] = "无需操作",
  [ROBOT_SYS_NOT_INIT] = "请调用robot_initialize()进行初始化",
  [ROBOT_SYS_INIT_FAILED] = "1.检查所有硬件连接\n2.重启系统\n3.联系技术支持",
  [ROBOT_MOTION_OVER_CURRENT] = "1.减小负载\n2.检查机械结构是否卡住\n3.检查电机驱动",
  [ROBOT_SAFE_EMERGENCY_STOP] = "1.检查急停按钮\n2.手动复位\n3.确认安全后重新启动",
  [ROBOT_UNKNOWN_ERROR] = "记录错误码并联系技术支持"
};

const char* robot_error_to_string(robot_error_t err) {
  if (err <= ROBOT_UNKNOWN_ERROR && robot_error_strings[err] != NULL) {
    return robot_error_strings[err];
  }

  // 未定义的错误码
  static char unknown_buf[32];
  snprintf(unknown_buf, sizeof(unknown_buf), "未定义错误 (0x%02X)", err);
  return unknown_buf;
}

const char* robot_error_get_suggestion(robot_error_t err) {
  if (err <= ROBOT_UNKNOWN_ERROR && robot_error_suggestions[err] != NULL) {
    return robot_error_suggestions[err];
  }

  // 根据模块提供通用建议
  robot_module_t module = robot_error_get_module(err);
  switch (module) {
    case ROBOT_MODULE_SAFETY:
      return "立即停止所有运动，确保安全后检查故障原因";
    case ROBOT_MODULE_POWER:
      return "检查电源连接和电池状态";
    case ROBOT_MODULE_MOTION:
      return "检查机械结构和电机驱动";
    default:
      return "尝试重启系统或联系技术支持";
  }
}

#else  // Release模式

// Release模式：最小化字符串存储
const char* robot_error_to_string(robot_error_t err) {
  static char error_buf[16];

  // 只处理最常见的错误
  switch (err) {
    case ROBOT_OK: return "成功";
    case ROBOT_SYS_NOT_INIT: return "未初始化";
    case ROBOT_SYS_INIT_FAILED: return "初始化失败";
    case ROBOT_SYS_TIMEOUT: return "超时";
    case ROBOT_SYS_NO_MEMORY: return "内存不足";
    case ROBOT_MOTION_OVER_CURRENT: return "过流";
    case ROBOT_MOTION_OVER_TEMP: return "过热";
    case ROBOT_SAFE_EMERGENCY_STOP: return "急停";
    case ROBOT_PWR_LOW_BATTERY: return "电量低";
    case ROBOT_UNKNOWN_ERROR: return "未知错误";
    default:
      // 按模块返回简短描述
      switch (robot_error_get_module(err)) {
        case ROBOT_MODULE_SAFETY: return "安全错误";
        case ROBOT_MODULE_POWER: return "电源错误";
        case ROBOT_MODULE_MOTION: return "运动错误";
        case ROBOT_MODULE_SENSOR: return "传感器错误";
        default: return "系统错误";
      }
  }
}

const char* robot_error_get_suggestion(robot_error_t err) {
  return "请参考用户手册或联系技术支持";
}

#endif // NDEBUG

// ==================== 公共函数实现 ====================

uint8_t robot_error_get_severity(robot_error_t err) {
  robot_module_t module = robot_error_get_module(err);

  // 安全错误最严重
  if (module == ROBOT_MODULE_SAFETY) {
    if (err == ROBOT_SAFE_EMERGENCY_STOP) return 4; // 致命
    return 3; // 严重
  }

  // 电源错误次之
  if (module == ROBOT_MODULE_POWER) {
    if ((err & 0x0F) >= 0x5) return 3; // 严重
    return 2; // 错误
  }

  // 运动控制错误
  if (module == ROBOT_MODULE_MOTION) {
    if (err == ROBOT_MOTION_STALL_DETECTED ||
        err == ROBOT_MOTION_OVER_CURRENT) {
      return 3; // 严重
    }
    return 2; // 错误
  }

  // 系统错误
  if (module == ROBOT_MODULE_SYSTEM) {
    if (err == ROBOT_SYS_HW_FAULT ||
        err == ROBOT_SYS_NO_MEMORY) {
      return 3; // 严重
    }
    return 2; // 错误
  }

  // 其他错误
  return 1; // 警告
}

bool robot_error_is_recoverable(robot_error_t err) {
  // 安全错误通常需要手动干预
  if (robot_error_get_module(err) == ROBOT_MODULE_SAFETY) {
    return false;
  }

  // 硬件故障不可恢复
  if (err == ROBOT_SYS_HW_FAULT ||
      err == ROBOT_MOTION_SERVO_FAULT) {
    return false;
  }

  // 其他错误通常可恢复
  return true;
}
