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

#ifndef AIRBORNE_DETECTOR_H
#define AIRBORNE_DETECTOR_H

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// 腾空状态
typedef enum {
  GROUND_CONTACT = 0, // 地面接触
  WHEELS_OFF_GROUND,  // 轮子离地（轻度腾空）
  FULLY_AIRBORNE,     // 完全腾空（跳跃）
  LANDING_IMPACT,     // 着陆冲击
  UNCERTAIN_STATE     // 不确定状态
} airborne_state_t;

// 检测算法类型
typedef enum {
  DETECT_ACCELERATION = 0, // 基于加速度
  DETECT_VIBRATION,        // 基于振动
  DETECT_COMBINED,         // 综合检测
  DETECT_ADAPTIVE          // 自适应检测
} detect_algorithm_t;

// IMU数据结构
typedef struct {
  float accel[3];      // 加速度 (m/s²) [x, y, z]
  float gyro[3];       // 角速度 (rad/s) [x, y, z]
  float quaternion[4]; // 四元数 (可选)
  uint32_t timestamp;  // 时间戳 (ms)
} imu_data_t;

// 腾空检测配置
typedef struct {
  detect_algorithm_t algorithm; // 检测算法
  float accel_threshold;        // 加速度阈值 (m/s²)
  float gyro_threshold;         // 角速度阈值 (rad/s)
  float variance_threshold;     // 方差阈值
  float impact_threshold;       // 冲击阈值 (m/s²)
  uint32_t min_airborne_time;   // 最小腾空时间 (ms)
  uint32_t max_airborne_time;   // 最大腾空时间 (ms)
  float confidence_threshold;   // 置信度阈值
  float gravity;                // 当地重力加速度
} airborne_config_t;

// 腾空检测器
typedef struct {
  airborne_state_t state;   // 当前状态
  airborne_config_t config; // 配置参数

  // 状态变量
  float vertical_acceleration; // 垂直加速度
  float total_acceleration;    // 合加速度
  float acceleration_variance; // 加速度方差
  float gyro_variance;         // 角速度方差
  float last_vertical_accel;   // 上次垂直加速度
  float free_fall_accel;       // 自由落体加速度阈值

  // 时间相关
  uint32_t airborne_start_time; // 腾空开始时间
  uint32_t state_timestamp;     // 状态时间戳
  uint32_t last_update_time;    // 最后更新时间

  // 滤波器和窗口
  float accel_filter_alpha;   // 加速度滤波系数
  float gyro_filter_alpha;    // 陀螺仪滤波系数
  float filtered_accel[3];    // 滤波后的加速度
  float filtered_gyro[3];     // 滤波后的角速度
  float accel_history[20][3]; // 加速度历史窗口
  int history_index;          // 历史索引

  // 统计信息
  uint32_t total_airborne_time; // 总腾空时间
  uint32_t airborne_count;      // 腾空次数
  float max_vertical_accel;     // 最大垂直加速度
  float max_impact_force;       // 最大冲击力
  float detection_confidence;   // 检测置信度
} airborne_detector_t;

// 初始化腾空检测器
void airborne_detector_init(airborne_detector_t* detector, const airborne_config_t* config);

// 更新检测器状态
airborne_state_t airborne_detector_update(airborne_detector_t* detector, const imu_data_t* imu);

// 获取当前状态
airborne_state_t airborne_detector_get_state(const airborne_detector_t* detector);

// 获取腾空时间 (ms)
uint32_t airborne_detector_get_airborne_duration(const airborne_detector_t* detector);

// 获取检测置信度 (0.0-1.0)
float airborne_detector_get_confidence(const airborne_detector_t* detector);

// 检查是否腾空
bool airborne_detector_is_airborne(const airborne_detector_t* detector);

// 检查是否正在着陆
bool airborne_detector_is_landing(const airborne_detector_t* detector);

// 获取垂直加速度
float airborne_detector_get_vertical_accel(const airborne_detector_t* detector);

// 估计腾空高度 (需要积分，不精确)
float airborne_detector_estimate_height(const airborne_detector_t* detector);

// 重置检测器
void airborne_detector_reset(airborne_detector_t* detector);

// 打印状态信息
void airborne_detector_print_status(const airborne_detector_t* detector);

#ifdef __cplusplus
}
#endif

#endif // AIRBORNE_DETECTOR_H
