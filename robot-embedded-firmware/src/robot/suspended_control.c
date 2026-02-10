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

// suspended_control.c

#include "robot/suspended_control.h"
#include <stdio.h>
#include <string.h>

// 初始化
void suspended_controller_init(suspended_controller_t* controller) {
  if (!controller) return;

  memset(controller, 0, sizeof(suspended_controller_t));

  // 初始化检测器
  multi_sensor_detector_init(&controller->detector);

  // 设置控制参数
  controller->params.stabilize_kp = 0.3f;
  controller->params.stabilize_kd = 0.1f;
  controller->params.descend_rate = 0.2f;
  controller->params.recover_power = 0.5f;
  controller->params.emergency_stop_time = 10000; // 10秒

  // 设置安全限制
  controller->safety.max_safe_time = 30000;    // 30秒
  controller->safety.max_tilt_angle = 30.0f;   // 30度
  controller->safety.max_descent_speed = 0.5f; // 0.5m/s
  controller->safety.enable_auto_shutdown = true;

  controller->mode = SUSPENDED_CTRL_NONE;
}

// 更新控制器
void suspended_controller_update(suspended_controller_t* controller,
  const sensor_data_t* sensors,
  uint8_t sensor_count) {
  if (!controller || !sensors) return;

  // 更新检测器状态
  suspended_state_t state = multi_sensor_detector_update(&controller->detector,
    sensors,
    sensor_count);

  uint32_t current_time = sensors[0].timestamp; // 假设第一个传感器有时间戳

  // 状态机
  suspended_control_mode_t old_mode = controller->mode;

  switch (controller->mode) {
    case SUSPENDED_CTRL_NONE:
      if (state >= SUSPENDED_STABLE) {
        // 进入悬空状态
        controller->mode = SUSPENDED_CTRL_STABILIZE;
        controller->status.mode_start_time = current_time;
        printf("进入悬空稳定模式\n");
      }
      break;

    case SUSPENDED_CTRL_STABILIZE:
      // 检查是否需要下降
      if (state == SUSPENDED_LONG_TERM) {
        controller->mode = SUSPENDED_CTRL_DESCEND;
        controller->status.mode_start_time = current_time;
        printf("进入缓慢下降模式\n");
      }
      // 检查是否恢复接地
      else if (state == SUSPENDED_NONE) {
        controller->mode = SUSPENDED_CTRL_NONE;
        printf("恢复地面控制\n");
      }
      break;

    case SUSPENDED_CTRL_DESCEND:
      // 检查是否需要恢复尝试
      if (current_time - controller->status.mode_start_time > 5000) {
        controller->mode = SUSPENDED_CTRL_RECOVER;
        controller->status.mode_start_time = current_time;
        controller->status.recovery_attempts++;
        printf("进入恢复尝试模式 (尝试#%lu)\n", controller->status.recovery_attempts);
      }
      // 检查是否恢复接地
      else if (state == SUSPENDED_NONE) {
        controller->mode = SUSPENDED_CTRL_NONE;
        printf("下降过程中恢复接地\n");
      }
      break;

    case SUSPENDED_CTRL_RECOVER:
      // 恢复尝试
      if (state == SUSPENDED_NONE) {
        controller->mode = SUSPENDED_CTRL_NONE;
        controller->status.recovery_successful = true;
        printf("恢复成功！\n");
      }
      // 检查是否需要紧急处理
      else if (current_time - controller->status.mode_start_time > 10000 ||
               controller->status.recovery_attempts >= 3) {
        controller->mode = SUSPENDED_CTRL_EMERGENCY;
        controller->status.mode_start_time = current_time;
        printf("进入紧急处理模式\n");
      }
      break;

    case SUSPENDED_CTRL_EMERGENCY:
      // 紧急处理
      if (current_time - controller->status.mode_start_time >
          controller->params.emergency_stop_time) {
        printf("紧急处理超时，系统关机\n");
        // 这里可以添加系统关机代码
      }
      break;
  }

  // 更新统计
  if (state >= SUSPENDED_STABLE) {
    controller->status.total_suspended_time += 10; // 假设10ms周期
    uint32_t duration = suspended_detector_get_duration(&controller->detector.imu_detector);
    if (duration > controller->status.max_suspended_time) {
      controller->status.max_suspended_time = duration;
    }
  }

  // 检查安全限制
  if (controller->safety.enable_auto_shutdown &&
      controller->status.total_suspended_time > controller->safety.max_safe_time) {
    printf("超过最大安全悬空时间，系统关机\n");
    controller->mode = SUSPENDED_CTRL_EMERGENCY;
    controller->status.mode_start_time = current_time;
  }
}

// 获取控制模式
suspended_control_mode_t suspended_controller_get_mode(const suspended_controller_t* controller) {
  return controller ? controller->mode : SUSPENDED_CTRL_NONE;
}

// 计算控制输出
void suspended_controller_calculate_output(const suspended_controller_t* controller,
  float* motor_left, float* motor_right) {
  if (!controller || !motor_left || !motor_right) return;

  // 默认输出
  *motor_left = 0.0f;
  *motor_right = 0.0f;

  // 根据控制模式计算输出
  switch (controller->mode) {
    case SUSPENDED_CTRL_NONE:
      // 正常地面控制
      *motor_left = 0.0f; // 由主控制器设置
      *motor_right = 0.0f;
      break;

    case SUSPENDED_CTRL_STABILIZE:
      // 姿态稳定控制
      // 简化的姿态稳定算法
      float pitch_error = get_pitch_error(); // 需要实际实现
      float roll_error = get_roll_error();   // 需要实际实现

      *motor_left = controller->params.stabilize_kp * pitch_error +
                    controller->params.stabilize_kd * roll_error;
      *motor_right = controller->params.stabilize_kp * pitch_error -
                     controller->params.stabilize_kd * roll_error;

      // 限制输出
      *motor_left = constrain(*motor_left, -0.3f, 0.3f);
      *motor_right = constrain(*motor_right, -0.3f, 0.3f);
      break;

    case SUSPENDED_CTRL_DESCEND:
      // 缓慢下降
      *motor_left = -controller->params.descend_rate;
      *motor_right = -controller->params.descend_rate;
      break;

    case SUSPENDED_CTRL_RECOVER:
      // 恢复尝试
      *motor_left = controller->params.recover_power;
      *motor_right = -controller->params.recover_power; // 产生旋转力矩
      break;

    case SUSPENDED_CTRL_EMERGENCY:
      // 紧急停止
      *motor_left = 0.0f;
      *motor_right = 0.0f;
      break;
  }
}

// 打印状态
void suspended_controller_print_status(const suspended_controller_t* controller) {
  if (!controller) return;

  const char* mode_str[] = {
    "无控制", "姿态稳定", "缓慢下降", "恢复尝试", "紧急处理"
  };

  printf("=== 悬空控制器状态 ===\n");
  printf("控制模式: %s\n", mode_str[controller->mode]);
  printf("悬空状态: %s\n", suspended_state_to_string(controller->detector.fused_state));
  printf("悬空时间: %lu ms\n", suspended_detector_get_duration(&controller->detector.imu_detector));
  printf("总悬空时间: %lu ms\n", controller->status.total_suspended_time);
  printf("最长悬空: %lu ms\n", controller->status.max_suspended_time);
  printf("恢复尝试: %lu\n", controller->status.recovery_attempts);
  printf("恢复成功: %s\n", controller->status.recovery_successful ? "是" : "否");
  printf("=====================\n\n");
}
