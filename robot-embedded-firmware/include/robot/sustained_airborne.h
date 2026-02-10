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

#ifndef SUSTAINED_AIRBORNE_H
#define SUSTAINED_AIRBORNE_H

#include <math.h>
#include "defs.h"

#ifdef __cplusplus
extern "C" {


#endif

// 悬空状态级别
typedef enum {
  SUSPENDED_NONE = 0,  // 未悬空（地面接触）
  SUSPENDED_TRANSIENT, // 瞬态悬空（短暂离地）
  SUSPENDED_STABLE,    // 稳定悬空（持续离地）
  SUSPENDED_LONG_TERM, // 长期悬空（被抬起或卡住）
  SUSPENDED_PERMANENT  // 永久悬空（脱离地面）
} suspended_state_t;

// 悬空检测算法
typedef enum {
  DETECTION_TIME_BASED = 0,  // 基于时间
  DETECTION_PATTERN_BASED,   // 基于模式
  DETECTION_FREQUENCY_BASED, // 基于频率
  DETECTION_ADAPTIVE         // 自适应
} suspended_detection_t;

// 悬空检测配置
typedef struct {
  suspended_detection_t algorithm; // 检测算法

  // 时间阈值 (ms)
  uint16_t transient_threshold; // 瞬态阈值 (50-200ms)
  uint16_t stable_threshold;    // 稳定阈值 (500-2000ms)
  uint32_t long_term_threshold; // 长期阈值 (3000-10000ms)

  // 模式匹配参数
  float pattern_similarity; // 模式相似度阈值
  uint16_t pattern_window;  // 模式窗口大小

  // 频率参数
  float vibration_frequency; // 振动频率阈值 (Hz)
  float frequency_stability; // 频率稳定性阈值

  // 适应性参数
  float adaptation_rate;        // 自适应速率
  uint32_t adaptation_interval; // 自适应间隔

  // 安全参数
  bool enable_auto_recovery;   // 启用自动恢复
  uint32_t max_suspended_time; // 最大悬空时间 (安全限制)
  float recovery_confidence;   // 恢复置信度阈值
} suspended_config_t;

// 悬空检测器
typedef struct {
  suspended_state_t state;   // 当前状态
  suspended_config_t config; // 配置

  // 时间相关
  uint32_t start_time;           // 悬空开始时间
  uint32_t duration;             // 当前悬空持续时间
  uint32_t total_suspended_time; // 累计悬空时间
  uint32_t last_ground_time;     // 最后着地时间

  // 传感器数据
  struct {
    float vertical_accel;   // 垂直加速度
    float vertical_jerk;    // 垂直加加速度
    float vibration_energy; // 振动能量
    float pattern[50];      // 模式数组
    uint16_t pattern_index; // 模式索引
  } sensor;

  // 频率分析
  struct {
    float dominant_frequency;  // 主导频率
    float frequency_stability; // 频率稳定性
    float fft_magnitude;       // FFT幅值
  } frequency;

  // 模式识别
  struct {
    float ground_pattern[50];    // 地面模式
    float suspended_pattern[50]; // 悬空模式
    float similarity;            // 当前相似度
  } pattern;

  // 检测置信度
  struct {
    float time_based;      // 基于时间的置信度
    float pattern_based;   // 基于模式的置信度
    float frequency_based; // 基于频率的置信度
    float overall;         // 总体置信度
  } confidence;

  // 状态历史
  struct {
    suspended_state_t history[10]; // 历史状态
    uint8_t index;                 // 索引
    uint8_t ground_count;          // 地面状态计数
    uint8_t suspended_count;       // 悬空状态计数
  } history;

  // 统计信息
  struct {
    uint32_t state_transitions; // 状态转换次数
    uint32_t false_positives;   // 误报次数
    uint32_t false_negatives;   // 漏报次数
    float avg_suspended_time;   // 平均悬空时间
    float max_suspended_time;   // 最长悬空时间
  } stats;

  // 控制标志
  bool is_calibrated;           // 是否已校准
  bool auto_recovery_triggered; // 自动恢复已触发
  uint32_t recovery_start_time; // 恢复开始时间
} suspended_detector_t;

// 初始化悬空检测器
void suspended_detector_init(suspended_detector_t*, const suspended_config_t* config);

// 更新检测器状态
suspended_state_t suspended_detector_update(suspended_detector_t*,
  float vertical_accel,
  float vertical_velocity,
  uint32_t timestamp);

// 获取当前状态
suspended_state_t suspended_detector_get_state(const suspended_detector_t*);

// 检查是否持续悬空
bool suspended_detector_is_suspended(const suspended_detector_t*);

// 获取悬空持续时间 (ms)
uint32_t suspended_detector_get_duration(const suspended_detector_t*);

// 获取检测置信度
float suspended_detector_get_confidence(const suspended_detector_t*);

// 校准地面模式（需要在地面静止时调用）
bool suspended_detector_calibrate_ground(suspended_detector_t*,
  float* accel_samples, uint16_t sample_count);

// 重置检测器
void suspended_detector_reset(suspended_detector_t*);

// 获取状态字符串
const char* suspended_state_to_string(suspended_state_t state);

// 获取建议的控制动作
const char* suspended_detector_get_recommendation(const suspended_detector_t*);


#ifdef __cplusplus
}
#endif

#endif // SUSTAINED_AIRBORNE_H
