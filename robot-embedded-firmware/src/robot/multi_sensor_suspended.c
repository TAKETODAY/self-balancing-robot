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

#include "robot/multi_sensor_suspended.h"
#include <stdio.h>
#include <string.h>

// 初始化
void multi_sensor_detector_init(multi_sensor_detector_t* detector) {
  if (!detector) return;

  memset(detector, 0, sizeof(multi_sensor_detector_t));

  // 初始化IMU检测器
  suspended_detector_init(&detector->imu_detector, NULL);

  // 设置默认权重
  detector->weights.imu_weight = 0.4f;
  detector->weights.motor_weight = 0.2f;
  detector->weights.encoder_weight = 0.15f;
  detector->weights.vibration_weight = 0.1f;
  detector->weights.ultrasonic_weight = 0.1f;
  detector->weights.pressure_weight = 0.05f;

  // 初始化校准状态
  detector->calibration.ground_current = 1.0f;   // 默认1A地面电流
  detector->calibration.ground_vibration = 0.5f; // 默认振动水平
  detector->calibration.ground_distance = 0.05f; // 默认5cm地面距离
}

// 校准传感器
void multi_sensor_detector_calibrate(multi_sensor_detector_t* detector,
  const sensor_data_t* sensor_data) {
  if (!detector || !sensor_data) return;

  for (int i = 0; i < 6; i++) {
    switch (sensor_data[i].type) {
      case SENSOR_IMU:
        // 校准IMU地面模式
      {
        float accel_samples[50];
        for (int j = 0; j < 50; j++) {
          accel_samples[j] = sensor_data[i].data.imu.accel[2]; // Z轴
        }
        suspended_detector_calibrate_ground(&detector->imu_detector,
          accel_samples, 50);
        detector->calibration.calibrated[0] = true;
      }
      break;

      case SENSOR_MOTOR_CURRENT:
        // 校准地面电流
        detector->calibration.ground_current =
            (sensor_data[i].data.motor.left + sensor_data[i].data.motor.right) / 2.0f;
        detector->calibration.calibrated[1] = true;
        break;

      case SENSOR_VIBRATION:
        // 校准地面振动
        detector->calibration.ground_vibration = sensor_data[i].data.vibration.amplitude;
        detector->calibration.calibrated[3] = true;
        break;

      case SENSOR_ULTRASONIC:
        // 校准地面距离
        detector->calibration.ground_distance = sensor_data[i].data.ultrasonic.distance;
        detector->calibration.calibrated[4] = true;
        break;

      default:
        break;
    }
  }

  printf("多传感器校准完成\n");
}

// 基于电机电流的悬空检测
static bool detect_by_motor_current(const multi_sensor_detector_t* detector,
  const sensor_data_t* sensor) {
  // 悬空时电流通常显著降低
  float avg_current = (sensor->data.motor.left + sensor->data.motor.right) / 2.0f;
  float ratio = avg_current / detector->calibration.ground_current;

  // 电流低于地面电流的30%，可能是悬空
  return (ratio < 0.3f && detector->calibration.calibrated[1]);
}

// 基于编码器的悬空检测
static bool detect_by_encoder(const multi_sensor_detector_t* detector,
  const sensor_data_t* sensor) {
  // 悬空时可能出现转速异常（滑移）
  // 这里简化处理：转速与指令不符且负载低
  float slip = sensor->data.encoder.slip;

  // 滑移率超过20%可能是悬空
  return (slip > 0.2f);
}

// 基于振动的悬空检测
static bool detect_by_vibration(const multi_sensor_detector_t* detector,
  const sensor_data_t* sensor) {
  if (!detector->calibration.calibrated[3]) return false;

  // 悬空时振动通常减小
  float vibration_ratio = sensor->data.vibration.amplitude /
                          detector->calibration.ground_vibration;

  return (vibration_ratio < 0.3f);
}

// 基于超声波的悬空检测
static bool detect_by_ultrasonic(const multi_sensor_detector_t* detector,
  const sensor_data_t* sensor) {
  if (!detector->calibration.calibrated[4]) return false;

  // 距离显著大于地面距离可能是悬空
  float distance_ratio = sensor->data.ultrasonic.distance /
                         detector->calibration.ground_distance;

  return (distance_ratio > 2.0f && sensor->data.ultrasonic.confidence > 0.7f);
}

// 基于压力的悬空检测
static bool detect_by_pressure(const multi_sensor_detector_t* detector,
  const sensor_data_t* sensor) {
  // 压力接近0可能是悬空
  return (sensor->data.pressure.force < 0.1f);
}

// 更新多传感器检测器
suspended_state_t multi_sensor_detector_update(multi_sensor_detector_t* detector,
  const sensor_data_t* sensors,
  uint8_t sensor_count) {
  if (!detector || !sensors || sensor_count == 0) {
    return SUSPENDED_NONE;
  }

  detector->detection_count++;

  // 处理每个传感器
  float imu_vertical_accel = 0.0f;
  uint32_t timestamp = 0;

  for (int i = 0; i < sensor_count; i++) {
    const sensor_data_t* sensor = &sensors[i];

    switch (sensor->type) {
      case SENSOR_IMU:
        // 更新IMU检测器
        imu_vertical_accel = sensor->data.imu.accel[2];
        timestamp = sensor->timestamp;

        suspended_detector_update(&detector->imu_detector,
          imu_vertical_accel,
          0, // 简化的垂直速度
          timestamp);
        break;

      case SENSOR_MOTOR_CURRENT:
        detector->sensor_states.motor_off_ground =
            detect_by_motor_current(detector, sensor);
        break;

      case SENSOR_ENCODER:
        detector->sensor_states.encoder_off_ground =
            detect_by_encoder(detector, sensor);
        break;

      case SENSOR_VIBRATION:
        detector->sensor_states.vibration_off_ground =
            detect_by_vibration(detector, sensor);
        break;

      case SENSOR_ULTRASONIC:
        detector->sensor_states.ultrasonic_off_ground =
            detect_by_ultrasonic(detector, sensor);
        break;

      case SENSOR_PRESSURE:
        detector->sensor_states.pressure_off_ground =
            detect_by_pressure(detector, sensor);
        break;

      default:
        break;
    }
  }

  // 传感器投票
  int vote_suspended = 0;
  int vote_grounded = 0;

  if (detector->imu_detector.state >= SUSPENDED_STABLE) vote_suspended++;
  else vote_grounded++;

  if (detector->sensor_states.motor_off_ground) vote_suspended++;
  else if (detector->calibration.calibrated[1]) vote_grounded++;

  if (detector->sensor_states.encoder_off_ground) vote_suspended++;
  else vote_grounded++;

  if (detector->sensor_states.vibration_off_ground) vote_suspended++;
  else if (detector->calibration.calibrated[3]) vote_grounded++;

  if (detector->sensor_states.ultrasonic_off_ground) vote_suspended++;
  else if (detector->calibration.calibrated[4]) vote_grounded++;

  if (detector->sensor_states.pressure_off_ground) vote_suspended++;
  else vote_grounded++;

  // 加权融合
  float imu_confidence = detector->imu_detector.confidence.overall;
  float motor_confidence = detector->sensor_states.motor_off_ground ? 0.8f : 0.2f;
  float encoder_confidence = detector->sensor_states.encoder_off_ground ? 0.7f : 0.3f;

  detector->fused_confidence =
      imu_confidence * detector->weights.imu_weight +
      motor_confidence * detector->weights.motor_weight +
      encoder_confidence * detector->weights.encoder_weight;

  // 确定融合状态
  if (vote_suspended >= 4) {
    // 大多数传感器认为悬空
    detector->fused_state = SUSPENDED_STABLE;
    detector->is_definitely_suspended = true;
    detector->is_definitely_grounded = false;
  }
  else if (vote_grounded >= 4) {
    // 大多数传感器认为接地
    detector->fused_state = SUSPENDED_NONE;
    detector->is_definitely_suspended = false;
    detector->is_definitely_grounded = true;
  }
  else {
    // 不确定，使用IMU状态
    detector->fused_state = detector->imu_detector.state;
    detector->is_definitely_suspended = false;
    detector->is_definitely_grounded = false;
  }

  // 检查悬空持续时间
  uint32_t duration = suspended_detector_get_duration(&detector->imu_detector);
  if (detector->fused_state >= SUSPENDED_STABLE && duration > 5000) {
    detector->fused_state = SUSPENDED_LONG_TERM;
  }

  // 统计
  bool actual_suspended = false; // 实际状态（需要地面真值）
  if ((detector->fused_state >= SUSPENDED_STABLE) == actual_suspended) {
    detector->correct_detections++;
  }
  else {
    detector->false_detections++;
  }

  return detector->fused_state;
}

// 是否持续悬空
bool multi_sensor_is_suspended(const multi_sensor_detector_t* detector) {
  if (!detector) return false;

  return (detector->fused_state >= SUSPENDED_STABLE);
}

// 打印状态
void multi_sensor_print_status(const multi_sensor_detector_t* detector) {
  if (!detector) return;

  printf("=== 多传感器悬空检测状态 ===\n");
  printf("融合状态: %s\n", suspended_state_to_string(detector->fused_state));
  printf("融合置信度: %.1f%%\n", detector->fused_confidence * 100);
  printf("悬空持续时间: %lu ms\n", suspended_detector_get_duration(&detector->imu_detector));

  printf("\n传感器状态:\n");
  printf("  IMU: %s\n", suspended_state_to_string(detector->imu_detector.state));
  printf("  电机: %s\n", detector->sensor_states.motor_off_ground ? "离地" : "接地");
  printf("  编码器: %s\n", detector->sensor_states.encoder_off_ground ? "离地" : "接地");
  printf("  振动: %s\n", detector->sensor_states.vibration_off_ground ? "离地" : "接地");
  printf("  超声波: %s\n", detector->sensor_states.ultrasonic_off_ground ? "离地" : "接地");
  printf("  压力: %s\n", detector->sensor_states.pressure_off_ground ? "离地" : "接地");

  printf("\n检测统计:\n");
  printf("  总检测次数: %lu\n", detector->detection_count);
  printf("  正确检测: %lu\n", detector->correct_detections);
  printf("  错误检测: %lu\n", detector->false_detections);
  printf("  准确率: %.1f%%\n",
    detector->detection_count > 0 ? (float) detector->correct_detections / detector->detection_count * 100 : 0);
  printf("============================\n\n");
}
