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

// main_with_mpu6050_suspended.c

#include <esp_timer.h>

#include "robot/attitude_to_suspended_adapter.h"
#include "attitude_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 全局变量
static mpu6050_suspended_adapter_t suspended_adapter;

static uint32_t last_status_print = 0;

// 控制函数示例
void enter_airborne_mode(void) {
  // 进入空中控制模式
  ESP_LOGI("CONTROL", "进入空中控制模式");

  // 1. 降低电机功率
  reduce_motor_power(0.3f);

  // 2. 切换到姿态保持模式
  switch_to_attitude_hold_mode();

  // 3. 准备着陆缓冲
  prepare_landing_buffer();
}

void attempt_ground_recovery(void) {
  // 尝试恢复地面接触
  ESP_LOGI("CONTROL", "尝试恢复地面接触");

  // 1. 缓慢增加下降速度
  increase_descent_speed(0.1f);

  // 2. 轻微调整姿态
  adjust_attitude_for_recovery();

  // 3. 如果超过20秒仍悬空，进入紧急模式
  uint32_t duration = mpu6050_suspended_adapter_get_duration(&suspended_adapter);
  if (duration > 20000) {
    ESP_LOGE("CONTROL", "恢复尝试失败，进入紧急模式");
    emergency_shutdown();
  }
}

void emergency_shutdown(void) {
  // 紧急关机
  ESP_LOGE("CONTROL", "执行紧急关机");

  // 1. 立即停止所有电机
  stop_all_motors();

  // 2. 保存当前状态
  save_current_state();

  // 3. 进入低功耗模式
  enter_low_power_mode();

  // 4. 发送紧急警报
  send_ble_emergency_alert("EMERGENCY_SHUTDOWN");
}

// 初始化函数
void suspended_detection_with_mpu6050_init(void) {
  ESP_LOGI("MAIN", "初始化MPU6050悬空检测系统...");

  // 初始化姿态传感器
  attitude_begin();

  // 初始化悬空检测适配器
  // 参数说明：
  // - accel_scale: 1.0表示默认量程±2g，如果是±4g则设为2.0，±8g设为4.0，±16g设为8.0
  // - gyro_scale: 1.0表示默认量程±250°/s，如果是±500°/s则设为2.0，±1000°/s设为4.0，±2000°/s设为8.0
  if (!mpu6050_suspended_adapter_init(&suspended_adapter, 1.0f, 1.0f)) {
    ESP_LOGE("MAIN", "悬空检测适配器初始化失败");
    return;
  }

  // 校准适配器（在地面静止时进行）
  ESP_LOGI("MAIN", "请将机器人放置在地面静止位置进行校准...");
  vTaskDelay(pdMS_TO_TICKS(3000));

  if (!mpu6050_suspended_adapter_calibrate(&suspended_adapter, 2000)) {
    ESP_LOGW("MAIN", "校准失败，但系统将继续运行");
  }

  ESP_LOGI("MAIN", "MPU6050悬空检测系统初始化完成");
}

// 主任务
void suspended_detection_task(void* arg) {
  (void) arg;

  suspended_detection_with_mpu6050_init();

  ESP_LOGI("MAIN", "开始悬空检测任务");

  while (1) {
    // 更新悬空检测
    suspended_state_t state = mpu6050_suspended_adapter_update(&suspended_adapter);

    // 获取当前时间
    uint32_t current_time = esp_timer_get_time() / 1000;

    // 状态变化检测
    static suspended_state_t last_state = SUSPENDED_NONE;
    if (state != last_state) {
      last_state = state;

      const char* state_str = suspended_state_to_string(state);
      float confidence = mpu6050_suspended_adapter_get_confidence(&suspended_adapter);

      ESP_LOGI("SUSPENDED", "状态变化: %s (置信度: %.1f%%)",
        state_str, confidence * 100);

      // 根据状态执行相应动作
      switch (state) {
        case SUSPENDED_TRANSIENT:
          // 瞬态悬空：保持平衡，准备着陆
          ESP_LOGI("CONTROL", "瞬态悬空 - 保持姿态");
          break;

        case SUSPENDED_STABLE:
          // 稳定悬空：进入空中控制模式
          ESP_LOGI("CONTROL", "稳定悬空 - 进入空中控制模式");
          enter_airborne_mode();
          break;

        case SUSPENDED_LONG_TERM:
          // 长期悬空：尝试恢复或降低功耗
          ESP_LOGW("CONTROL", "长期悬空 - 尝试恢复地面接触");
          attempt_ground_recovery();
          break;

        case SUSPENDED_PERMANENT:
          // 永久悬空：紧急处理
          ESP_LOGE("CONTROL", "永久悬空 - 需要人工干预！");
          emergency_shutdown();
          break;

        case SUSPENDED_NONE:
        default:
          // 地面接触：恢复正常控制
          ESP_LOGI("CONTROL", "地面接触 - 恢复正常控制");
          break;
      }
    }

    // 定期打印状态
    if (current_time - last_status_print > 2000) {
      // 每2秒打印一次
      last_status_print = current_time;
      mpu6050_suspended_adapter_print_status(&suspended_adapter);
    }

    // 检查是否一直悬空（超过5秒）
    if (mpu6050_suspended_adapter_is_suspended(&suspended_adapter)) {
      uint32_t duration = mpu6050_suspended_adapter_get_duration(&suspended_adapter);

      if (duration > 5000) {
        ESP_LOGW("SUSPENDED", "持续悬空超过5秒，可能需要人工干预");

        // 如果超过10秒，发送警报
        if (duration > 10000) {
          ESP_LOGE("SUSPENDED", "持续悬空超过10秒！");
          send_ble_alert("PROLONGED_SUSPENSION");
        }
      }
    }

    // 10ms控制周期
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


// 启动系统
void app_main2() {
  ESP_LOGI("MAIN", "启动机器人悬空检测系统");

  // 创建悬空检测任务
  xTaskCreate(suspended_detection_task,
    "suspended_detect",
    4096,
    NULL,
    5,
    NULL);

  // 这里可以添加其他任务
  ESP_LOGI("MAIN", "系统启动完成");
}
