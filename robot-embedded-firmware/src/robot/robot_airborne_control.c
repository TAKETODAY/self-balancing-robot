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

// robot_airborne_control.c
#include "fused_airborne_detector.h"
#include "esp_log.h"

// 机器人空中控制器
typedef struct {
  fused_airborne_detector_t detector; // 融合检测器

  // 控制参数
  float airborne_kp; // 空中控制P增益
  float airborne_kd; // 空中控制D增益
  float landing_kp; // 着陆控制P增益
  float landing_kd; // 着陆控制D增益
  float recovery_time; // 恢复时间 (ms)

  // 状态机
  enum {
    CONTROL_GROUND = 0, // 地面控制模式
    CONTROL_AIRBORNE, // 空中控制模式
    CONTROL_LANDING, // 着陆控制模式
    CONTROL_RECOVERY // 恢复控制模式
  } control_mode;

  // 时间管理
  uint32_t mode_start_time; // 模式开始时间
  uint32_t last_mode_change_time; // 最后模式切换时间

  // 性能统计
  uint32_t total_airborne_time; // 总腾空时间
  uint32_t max_airborne_time; // 最长腾空时间
  uint32_t hard_landings; // 硬着陆次数
  uint32_t successful_jumps; // 成功跳跃次数
} robot_airborne_control_t;

// 初始化
void robot_airborne_control_init(robot_airborne_control_t* controller) {
  if (!controller) return;

  memset(controller, 0, sizeof(robot_airborne_control_t));
  fused_airborne_detector_init(&controller->detector);

  // 默认控制参数
  controller->airborne_kp = 0.3f;
  controller->airborne_kd = 0.1f;
  controller->landing_kp = 0.8f;
  controller->landing_kd = 0.3f;
  controller->recovery_time = 200; // 200ms恢复时间

  controller->control_mode = CONTROL_GROUND;
  controller->mode_start_time = 0;
}

// 校准控制器（机器人放置在地面静止时调用）
void robot_airborne_calibrate(robot_airborne_control_t* controller,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor) {
  if (!controller) return;

  fused_airborne_detector_calibrate(&controller->detector, imu, left_motor, right_motor);
  ESP_LOGI("AIRBORNE", "机器人空中控制器校准完成");
}

// 更新控制器状态
void robot_airborne_control_update(robot_airborne_control_t* controller,
  const imu_data_t* imu,
  const motor_state_t* left_motor,
  const motor_state_t* right_motor) {
  if (!controller || !imu) return;

  static uint32_t last_update_time = 0;
  uint32_t current_time = imu->timestamp;

  // 更新检测器
  bool is_airborne = fused_airborne_detector_update(&controller->detector,
    imu, left_motor, right_motor);

  // 获取详细状态
  airborne_state_t state = controller->detector.fused_state;
  float confidence = fused_airborne_detector_get_confidence(&controller->detector);

  // 状态机更新
  switch (controller->control_mode) {
    case CONTROL_GROUND:
      if (is_airborne && confidence > 0.7f) {
        // 进入空中模式
        controller->control_mode = CONTROL_AIRBORNE;
        controller->mode_start_time = current_time;
        controller->last_mode_change_time = current_time;

        ESP_LOGI("AIRBORNE", "进入空中模式 (置信度: %.1f%%)", confidence * 100);

        // 执行离地动作
        execute_takeoff_sequence();
      }
      break;

    case CONTROL_AIRBORNE:
      if (!is_airborne || state == LANDING_IMPACT) {
        // 着陆检测
        controller->control_mode = CONTROL_LANDING;
        controller->mode_start_time = current_time;

        uint32_t airborne_duration = current_time - controller->last_mode_change_time;
        controller->total_airborne_time += airborne_duration;

        if (airborne_duration > controller->max_airborne_time) {
          controller->max_airborne_time = airborne_duration;
        }

        ESP_LOGI("AIRBORNE", "检测到着陆，腾空时间: %ums", airborne_duration);

        // 执行着陆动作
        execute_landing_sequence();
      }
      else {
        // 空中控制
        uint32_t airborne_duration = current_time - controller->last_mode_change_time;

        // 每100ms记录一次
        if (current_time - last_update_time > 100) {
          last_update_time = current_time;

          float height = airborne_detector_estimate_height(&controller->detector.imu_detector);
          ESP_LOGD("AIRBORNE", "空中: 时间=%ums, 高度=%.2fm, 置信度=%.1f%%",
            airborne_duration, height, confidence * 100);
        }

        // 执行空中控制
        control_airborne_attitude();
      }
      break;

    case CONTROL_LANDING:
      if (current_time - controller->mode_start_time > controller->recovery_time) {
        // 着陆后恢复
        controller->control_mode = CONTROL_RECOVERY;
        controller->mode_start_time = current_time;

        // 检查着陆质量
        float impact_force = controller->detector.imu_detector.max_impact_force;
        if (impact_force > 2.0f * controller->detector.imu_detector.config.gravity) {
          controller->hard_landings++;
          ESP_LOGW("AIRBORNE", "硬着陆! 冲击力: %.1fg",
            impact_force / controller->detector.imu_detector.config.gravity);
        }
        else {
          controller->successful_jumps++;
        }

        ESP_LOGI("AIRBORNE", "着陆恢复中...");
      }
      break;

    case CONTROL_RECOVERY:
      if (current_time - controller->mode_start_time > 300) {
        // 300ms恢复时间
        controller->control_mode = CONTROL_GROUND;
        ESP_LOGI("AIRBORNE", "恢复地面控制");
      }
      break;
  }
}

// 获取当前控制模式
const char* robot_airborne_get_mode_string(const robot_airborne_control_t* controller) {
  if (!controller) return "未知";

  switch (controller->control_mode) {
    case CONTROL_GROUND: return "地面控制";
    case CONTROL_AIRBORNE: return "空中控制";
    case CONTROL_LANDING: return "着陆控制";
    case CONTROL_RECOVERY: return "恢复控制";
    default: return "未知模式";
  }
}

// 控制输出计算
void robot_airborne_calculate_control(robot_airborne_control_t* controller,
  float* pitch_output,
  float* roll_output,
  float* throttle_output) {
  if (!controller || !pitch_output || !roll_output || !throttle_output) return;

  // 根据控制模式输出不同的控制量
  switch (controller->control_mode) {
    case CONTROL_GROUND:
      // 地面控制：正常平衡控制
      *pitch_output = calculate_pitch_control();
      *roll_output = calculate_roll_control();
      *throttle_output = 0; // 地面不需要油门
      break;

    case CONTROL_AIRBORNE:
      // 空中控制：姿态保持
      *pitch_output = controller->airborne_kp * get_pitch_error();
      *roll_output = controller->airborne_kd * get_roll_error();
      *throttle_output = 0.3f; // 轻微油门保持高度
      break;

    case CONTROL_LANDING:
      // 着陆控制：减震和稳定
      *pitch_output = controller->landing_kp * get_pitch_error();
      *roll_output = controller->landing_kd * get_roll_error();
      *throttle_output = -0.5f; // 反向推力减速
      break;

    case CONTROL_RECOVERY:
      // 恢复控制：缓慢回到平衡
      *pitch_output = 0.5f * calculate_pitch_control();
      *roll_output = 0.5f * calculate_roll_control();
      *throttle_output = 0;
      break;
  }
}

// 获取性能统计
void robot_airborne_get_stats(const robot_airborne_control_t* controller,
  uint32_t* total_airborne_time,
  uint32_t* max_airborne_time,
  uint32_t* successful_jumps,
  uint32_t* hard_landings) {
  if (!controller) return;

  if (total_airborne_time) *total_airborne_time = controller->total_airborne_time;
  if (max_airborne_time) *max_airborne_time = controller->max_airborne_time;
  if (successful_jumps) *successful_jumps = controller->successful_jumps;
  if (hard_landings) *hard_landings = controller->hard_landings;
}

// 执行离地序列
static void execute_takeoff_sequence(void) {
  // 1. 停止主动平衡控制
  disable_active_balancing();

  // 2. 记录离地时的姿态
  save_takeoff_attitude();

  // 3. 减小电机功率（防止空中过冲）
  reduce_motor_power(0.5f);

  // 4. 准备空中姿态控制
  initialize_airborne_control();

  ESP_LOGI("AIRBORNE", "离地序列执行完成");
}

// 执行着陆序列
static void execute_landing_sequence(void) {
  // 1. 预测着陆姿态
  predict_landing_attitude();

  // 2. 准备减震
  prepare_shock_absorption();

  // 3. 增加电机阻尼
  increase_motor_damping();

  // 4. 激活着陆保护
  activate_landing_protection();

  ESP_LOGI("AIRBORNE", "着陆序列执行完成");
}
