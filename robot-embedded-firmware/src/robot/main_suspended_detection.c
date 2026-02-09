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

// main_suspended_detection.c

#include "robot/suspended_control.h"
#include "robot/imu_driver.h"
#include "robot/motor_driver.h"
#include "robot/vibration_sensor.h"
#include "robot/ultrasonic_sensor.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 全局控制器
static suspended_controller_t suspended_ctrl;
static uint32_t last_status_print = 0;

// 初始化系统
void suspended_detection_system_init(void) {
  ESP_LOGI("SUSPENDED", "初始化悬空检测系统...");

  suspended_controller_init(&suspended_ctrl);

  // 初始化传感器
  imu_init();
  vibration_sensor_init();
  ultrasonic_init();

  ESP_LOGI("SUSPENDED", "悬空检测系统初始化完成");
}

// 校准系统
void calibrate_suspended_system(void) {
  ESP_LOGI("SUSPENDED", "开始系统校准...");
  printf("请将机器人放置在地面静止位置\n");
  vTaskDelay(pdMS_TO_TICKS(3000));

  // 收集校准数据
  sensor_data_t calibration_data[4] = { 0 };

  // IMU数据
  float accel[3], gyro[3];
  imu_read(accel, gyro);
  calibration_data[0].type = SENSOR_IMU;
  calibration_data[0].data.imu.accel[0] = accel[0];
  calibration_data[0].data.imu.accel[1] = accel[1];
  calibration_data[0].data.imu.accel[2] = accel[2];
  calibration_data[0].data.imu.gyro[0] = gyro[0];
  calibration_data[0].data.imu.gyro[1] = gyro[1];
  calibration_data[0].data.imu.gyro[2] = gyro[2];
  calibration_data[0].timestamp = esp_timer_get_time() / 1000;

  // 电机电流
  calibration_data[1].type = SENSOR_MOTOR_CURRENT;
  calibration_data[1].data.motor.left = motor_get_current(MOTOR_LEFT);
  calibration_data[1].data.motor.right = motor_get_current(MOTOR_RIGHT);
  calibration_data[1].timestamp = esp_timer_get_time() / 1000;

  // 振动数据
  calibration_data[2].type = SENSOR_VIBRATION;
  calibration_data[2].data.vibration.amplitude = vibration_get_amplitude();
  calibration_data[2].data.vibration.frequency = vibration_get_frequency();
  calibration_data[2].timestamp = esp_timer_get_time() / 1000;

  // 超声波数据
  calibration_data[3].type = SENSOR_ULTRASONIC;
  calibration_data[3].data.ultrasonic.distance = ultrasonic_get_distance();
  calibration_data[3].data.ultrasonic.confidence = ultrasonic_get_confidence();
  calibration_data[3].timestamp = esp_timer_get_time() / 1000;

  // 执行校准
  multi_sensor_detector_calibrate(&suspended_ctrl.detector, calibration_data);

  ESP_LOGI("SUSPENDED", "系统校准完成");
}

// 主检测任务
void suspended_detection_task(void* arg) {
  (void) arg;

  suspended_detection_system_init();
  calibrate_suspended_system();

  while (1) {
    // 读取所有传感器数据
    sensor_data_t sensors[5] = { 0 };
    uint32_t timestamp = esp_timer_get_time() / 1000;

    // IMU数据
    float accel[3], gyro[3];
    imu_read(accel, gyro);
    sensors[0].type = SENSOR_IMU;
    sensors[0].data.imu.accel[0] = accel[0];
    sensors[0].data.imu.accel[1] = accel[1];
    sensors[0].data.imu.accel[2] = accel[2];
    sensors[0].data.imu.gyro[0] = gyro[0];
    sensors[0].data.imu.gyro[1] = gyro[1];
    sensors[0].data.imu.gyro[2] = gyro[2];
    sensors[0].timestamp = timestamp;
    sensors[0].reliability = 0.9f;

    // 电机电流
    sensors[1].type = SENSOR_MOTOR_CURRENT;
    sensors[1].data.motor.left = motor_get_current(MOTOR_LEFT);
    sensors[1].data.motor.right = motor_get_current(MOTOR_RIGHT);
    sensors[1].timestamp = timestamp;
    sensors[1].reliability = 0.8f;

    // 编码器数据
    sensors[2].type = SENSOR_ENCODER;
    sensors[2].data.encoder.speed = encoder_get_speed();
    sensors[2].data.encoder.slip = encoder_get_slip();
    sensors[2].timestamp = timestamp;
    sensors[2].reliability = 0.7f;

    // 振动数据
    sensors[3].type = SENSOR_VIBRATION;
    sensors[3].data.vibration.amplitude = vibration_get_amplitude();
    sensors[3].data.vibration.frequency = vibration_get_frequency();
    sensors[3].timestamp = timestamp;
    sensors[3].reliability = 0.6f;

    // 超声波数据
    sensors[4].type = SENSOR_ULTRASONIC;
    sensors[4].data.ultrasonic.distance = ultrasonic_get_distance();
    sensors[4].data.ultrasonic.confidence = ultrasonic_get_confidence();
    sensors[4].timestamp = timestamp;
    sensors[4].reliability = 0.5f;

    // 更新悬空控制器
    suspended_controller_update(&suspended_ctrl, sensors, 5);

    // 根据控制模式调整电机输出
    float motor_left, motor_right;
    suspended_controller_calculate_output(&suspended_ctrl, &motor_left, &motor_right);

    // 应用控制输出（如果不在正常模式）
    if (suspended_controller_get_mode(&suspended_ctrl) != SUSPENDED_CTRL_NONE) {
      motor_set_power(MOTOR_LEFT, motor_left);
      motor_set_power(MOTOR_RIGHT, motor_right);
    }

    // 定期打印状态
    if (timestamp - last_status_print > 2000) {
      last_status_print = timestamp;
      suspended_controller_print_status(&suspended_ctrl);
      multi_sensor_print_status(&suspended_ctrl.detector);
    }

    // 检查是否需要紧急处理
    if (suspended_ctrl.detector.fused_state == SUSPENDED_PERMANENT) {
      ESP_LOGE("SUSPENDED", "检测到永久悬空状态，需要人工干预！");
      // 发送警报
      ble_send_alert("PERMANENT_SUSPENSION");
    }

    // 10ms周期
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// 启动悬空检测系统
void start_suspended_detection(void) {
  xTaskCreate(suspended_detection_task,
    "suspended_detect",
    4096,
    NULL,
    5,
    NULL);
}
