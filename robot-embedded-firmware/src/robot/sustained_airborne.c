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

// sustained_airborne.c

#include "robot/sustained_airborne.h"
#include <string.h>

// 默认配置
static const suspended_config_t DEFAULT_CONFIG = {
  .algorithm = DETECTION_ADAPTIVE,

  // 时间阈值 (ms)
  .transient_threshold = 100,  // 100ms瞬态
  .stable_threshold = 1000,    // 1秒稳定
  .long_term_threshold = 5000, // 5秒长期

  // 模式参数
  .pattern_similarity = 0.85f,
  .pattern_window = 50,

  // 频率参数
  .vibration_frequency = 5.0f, // 5Hz振动阈值
  .frequency_stability = 0.3f,

  // 自适应参数
  .adaptation_rate = 0.1f,
  .adaptation_interval = 5000,

  // 安全参数
  .enable_auto_recovery = true,
  .max_suspended_time = 30000, // 30秒最大悬空
  .recovery_confidence = 0.7f
};

// 初始化
void suspended_detector_init(suspended_detector_t* detector,
  const suspended_config_t* config) {
  if (!detector) return;

  memset(detector, 0, sizeof(suspended_detector_t));

  if (config) {
    detector->config = *config;
  }
  else {
    detector->config = DEFAULT_CONFIG;
  }

  detector->state = SUSPENDED_NONE;
  detector->is_calibrated = false;

  // 初始化模式数组
  for (int i = 0; i < 50; i++) {
    detector->pattern.ground_pattern[i] = 9.81f; // 默认重力加速度
  }
}

// 计算模式相似度（简单的余弦相似度）
static float calculate_similarity(const float* pattern1,
  const float* pattern2,
  uint16_t length) {
  float dot_product = 0.0f;
  float norm1 = 0.0f;
  float norm2 = 0.0f;

  for (uint16_t i = 0; i < length; i++) {
    dot_product += pattern1[i] * pattern2[i];
    norm1 += pattern1[i] * pattern1[i];
    norm2 += pattern2[i] * pattern2[i];
  }

  if (norm1 == 0 || norm2 == 0) return 0.0f;

  return dot_product / (sqrtf(norm1) * sqrtf(norm2));
}

// 更新模式数组
static void update_pattern(suspended_detector_t* detector, float value) {
  detector->sensor.pattern[detector->sensor.pattern_index] = value;
  detector->sensor.pattern_index =
      (detector->sensor.pattern_index + 1) % detector->config.pattern_window;
}

// 简单频率分析（移动平均法）
static void analyze_frequency(suspended_detector_t* detector, float accel, uint32_t dt_ms) {
  static float last_accel = 0.0f;
  static float last_velocity = 0.0f;
  static uint32_t zero_crossings = 0;
  static uint32_t sample_count = 0;
  static float sum_periods = 0.0f;

  float dt = dt_ms / 1000.0f;
  if (dt <= 0) dt = 0.001f;

  // 计算加加速度
  float jerk = (accel - last_accel) / dt;
  detector->sensor.vertical_jerk = jerk;

  // 计算振动能量
  detector->sensor.vibration_energy =
      0.9f * detector->sensor.vibration_energy + 0.1f * fabsf(jerk);

  // 简单过零检测
  if (last_accel * accel < 0) {
    zero_crossings++;
  }

  sample_count++;

  // 每100个样本计算一次频率
  if (sample_count >= 100) {
    if (zero_crossings > 2) {
      float avg_period = (sample_count * dt) / (zero_crossings / 2.0f);
      detector->frequency.dominant_frequency = 1.0f / avg_period;

      // 更新频率稳定性
      sum_periods += avg_period;
      float avg_period_all = sum_periods / (sample_count / 100.0f);
      detector->frequency.frequency_stability = 1.0f -
                                                fabsf(avg_period - avg_period_all) / avg_period_all;
    }
    else {
      detector->frequency.dominant_frequency = 0.0f;
      detector->frequency.frequency_stability = 0.0f;
    }

    // 重置计数器
    zero_crossings = 0;
    sample_count = 0;
  }

  last_accel = accel;
}

// 基于时间的检测算法
static float time_based_detection(const suspended_detector_t* this) {
  if (this->duration < this->config.transient_threshold) {
    return 0.0f; // 太短，不是悬空
  }

  float confidence = 0.0f;

  // 线性映射：100ms -> 0%，1s -> 50%，5s -> 100%
  if (this->duration <= this->config.stable_threshold) {
    confidence = (float) (this->duration - this->config.transient_threshold) /
                 (float) (this->config.stable_threshold - this->config.transient_threshold) * 0.5f;
  }
  else if (this->duration <= this->config.long_term_threshold) {
    confidence = 0.5f + (float) (this->duration - this->config.stable_threshold) /
                 (float) (this->config.long_term_threshold - this->config.stable_threshold) * 0.5f;
  }
  else {
    confidence = 1.0f;
  }

  return confidence;
}

// 基于模式的检测算法
static float pattern_based_detection(suspended_detector_t* detector) {
  if (!detector->is_calibrated) {
    return 0.5f; // 未校准，中等置信度
  }

  // 计算与地面模式的相似度
  float similarity = calculate_similarity(
    detector->sensor.pattern,
    detector->pattern.ground_pattern,
    detector->config.pattern_window
  );

  detector->pattern.similarity = similarity;

  // 相似度越低，悬空置信度越高
  return 1.0f - similarity;
}

// 基于频率的检测算法
static float frequency_based_detection(suspended_detector_t* detector) {
  // 悬空时振动频率通常较低
  float freq_score = 0.0f;
  if (detector->frequency.dominant_frequency > 0) {
    // 频率越高，悬空可能性越低
    freq_score = 1.0f - fminf(detector->frequency.dominant_frequency /
                              detector->config.vibration_frequency, 1.0f);
  }

  // 频率稳定性
  float stability_score = 1.0f - detector->frequency.frequency_stability;

  // 振动能量
  float energy_score = 1.0f - fminf(detector->sensor.vibration_energy / 10.0f, 1.0f);

  // 加权平均
  return (freq_score * 0.4f + stability_score * 0.3f + energy_score * 0.3f);
}

// 自适应检测算法
static float adaptive_detection(suspended_detector_t* detector) {
  // 获取各种检测算法的置信度
  float time_confidence = time_based_detection(detector);
  float pattern_confidence = pattern_based_detection(detector);
  float freq_confidence = frequency_based_detection(detector);

  // 根据悬空时间调整权重
  float time_weight, pattern_weight, freq_weight;

  if (detector->duration < 500) {
    // 短时间：主要依赖频率和模式
    time_weight = 0.2f;
    pattern_weight = 0.4f;
    freq_weight = 0.4f;
  }
  else if (detector->duration < 2000) {
    // 中等时间：均衡权重
    time_weight = 0.3f;
    pattern_weight = 0.35f;
    freq_weight = 0.35f;
  }
  else {
    // 长时间：主要依赖时间
    time_weight = 0.6f;
    pattern_weight = 0.2f;
    freq_weight = 0.2f;
  }

  // 计算总体置信度
  float overall = time_confidence * time_weight +
                  pattern_confidence * pattern_weight +
                  freq_confidence * freq_weight;

  // 保存各置信度
  detector->confidence.time_based = time_confidence;
  detector->confidence.pattern_based = pattern_confidence;
  detector->confidence.frequency_based = freq_confidence;
  detector->confidence.overall = overall;

  return overall;
}

// 状态转换逻辑
static suspended_state_t determine_state(suspended_detector_t* detector,
  float confidence,
  uint32_t duration) {
  suspended_state_t new_state = SUSPENDED_NONE;

  // 使用置信度和时间共同决定状态
  if (confidence < 0.3f) {
    new_state = SUSPENDED_NONE;
  }
  else if (confidence < 0.6f && duration < detector->config.stable_threshold) {
    new_state = SUSPENDED_TRANSIENT;
  }
  else if (confidence >= 0.6f && duration < detector->config.long_term_threshold) {
    new_state = SUSPENDED_STABLE;
  }
  else if (confidence >= 0.6f && duration >= detector->config.long_term_threshold) {
    new_state = SUSPENDED_LONG_TERM;
  }
  else if (confidence >= 0.8f && duration >= detector->config.max_suspended_time) {
    new_state = SUSPENDED_PERMANENT;
  }

  // 状态转换检测
  if (new_state != detector->state) {
    detector->stats.state_transitions++;

    if (detector->state == SUSPENDED_NONE && new_state != SUSPENDED_NONE) {
      // 开始悬空
      detector->start_time = detector->last_ground_time + duration;
    }
    else if (new_state == SUSPENDED_NONE) {
      // 恢复地面接触
      detector->last_ground_time = detector->start_time + duration;
      detector->total_suspended_time += duration;

      // 更新统计
      if (duration > detector->stats.max_suspended_time) {
        detector->stats.max_suspended_time = duration;
      }
      detector->stats.avg_suspended_time =
      (detector->stats.avg_suspended_time * (detector->stats.state_transitions - 1) +
       duration) / detector->stats.state_transitions;
    }
  }

  return new_state;
}

// 更新检测器
suspended_state_t suspended_detector_update(suspended_detector_t* detector,
  float vertical_accel,
  float vertical_velocity,
  uint32_t timestamp) {
  if (!detector) return SUSPENDED_NONE;

  static uint32_t last_timestamp = 0;
  uint32_t dt_ms = timestamp - last_timestamp;
  last_timestamp = timestamp;

  // 更新传感器数据
  detector->sensor.vertical_accel = vertical_accel;
  update_pattern(detector, vertical_accel);

  // 频率分析
  analyze_frequency(detector, vertical_accel, dt_ms);

  // 检查是否是地面接触状态
  bool likely_ground = false;

  // 地面接触的典型特征：
  // 1. 加速度接近重力（考虑±阈值）
  // 2. 有高频振动
  // 3. 垂直速度接近0
  float gravity_diff = fabsf(vertical_accel - 9.81f);
  if (gravity_diff < 0.5f &&
      detector->sensor.vibration_energy > 0.1f &&
      fabsf(vertical_velocity) < 0.1f) {
    likely_ground = true;
  }

  // 更新悬空持续时间
  if (likely_ground) {
    detector->duration = 0;
    detector->start_time = timestamp;
  }
  else {
    if (detector->duration == 0) {
      // 刚开始悬空
      detector->start_time = timestamp;
    }
    detector->duration = timestamp - detector->start_time;
  }

  // 根据算法计算置信度
  float confidence = 0.0f;

  switch (detector->config.algorithm) {
    case DETECTION_TIME_BASED:
      confidence = time_based_detection(detector);
      break;
    case DETECTION_PATTERN_BASED:
      confidence = pattern_based_detection(detector);
      break;
    case DETECTION_FREQUENCY_BASED:
      confidence = frequency_based_detection(detector);
      break;
    case DETECTION_ADAPTIVE:
    default:
      confidence = adaptive_detection(detector);
      break;
  }

  // 确定新状态
  suspended_state_t old_state = detector->state;
  detector->state = determine_state(detector, confidence, detector->duration);

  // 更新状态历史
  detector->history.history[detector->history.index] = detector->state;
  detector->history.index = (detector->history.index + 1) % 10;

  // 统计历史状态
  detector->history.ground_count = 0;
  detector->history.suspended_count = 0;
  for (int i = 0; i < 10; i++) {
    if (detector->history.history[i] == SUSPENDED_NONE) {
      detector->history.ground_count++;
    }
    else {
      detector->history.suspended_count++;
    }
  }

  // 检查是否需要自动恢复
  if (detector->config.enable_auto_recovery &&
      detector->state >= SUSPENDED_LONG_TERM &&
      !detector->auto_recovery_triggered) {

    if (detector->duration > detector->config.max_suspended_time) {
      detector->auto_recovery_triggered = true;
      detector->recovery_start_time = timestamp;
    }
  }

  return detector->state;
}

// 获取当前状态
suspended_state_t suspended_detector_get_state(const suspended_detector_t* this) {
  return this ? this->state : SUSPENDED_NONE;
}

// 检查是否持续悬空
bool suspended_detector_is_suspended(const suspended_detector_t* detector) {
  if (!detector) return false;

  // 稳定悬空或更高级别被认为是持续悬空
  return (detector->state >= SUSPENDED_STABLE);
}

// 获取悬空持续时间
uint32_t suspended_detector_get_duration(const suspended_detector_t* detector) {
  return detector ? detector->duration : 0;
}

// 获取检测置信度
float suspended_detector_get_confidence(const suspended_detector_t* detector) {
  return detector ? detector->confidence.overall : 0.0f;
}

// 校准地面模式
bool suspended_detector_calibrate_ground(suspended_detector_t* this, float* accel_samples, uint16_t sample_count) {
  if (!this || !accel_samples || sample_count == 0) {
    return false;
  }

  // 确保有足够样本
  uint16_t actual_count = (sample_count > 50) ? 50 : sample_count;

  // 计算平均值
  float sum = 0.0f;
  for (uint16_t i = 0; i < actual_count; i++) {
    sum += accel_samples[i];
  }
  float avg = sum / actual_count;

  // 保存到地面模式
  for (int i = 0; i < 50; i++) {
    if (i < actual_count) {
      this->pattern.ground_pattern[i] = accel_samples[i];
    }
    else {
      this->pattern.ground_pattern[i] = avg;
    }
  }

  this->is_calibrated = true;

  return true;
}

// 重置检测器
void suspended_detector_reset(suspended_detector_t* detector) {
  if (!detector) return;

  detector->state = SUSPENDED_NONE;
  detector->duration = 0;
  detector->start_time = 0;
  detector->confidence.overall = 0.0f;
  detector->auto_recovery_triggered = false;
}

// 获取状态字符串
const char* suspended_state_to_string(suspended_state_t state) {
  switch (state) {
    case SUSPENDED_NONE: return "地面接触";
    case SUSPENDED_TRANSIENT: return "瞬态悬空";
    case SUSPENDED_STABLE: return "稳定悬空";
    case SUSPENDED_LONG_TERM: return "长期悬空";
    case SUSPENDED_PERMANENT: return "永久悬空";
    default: return "未知状态";
  }
}

// 获取建议的控制动作
const char* suspended_detector_get_recommendation(const suspended_detector_t* detector) {
  if (!detector) return "无建议";

  switch (detector->state) {
    case SUSPENDED_NONE:
      return "正常平衡控制";
    case SUSPENDED_TRANSIENT:
      return "准备着陆，保持姿态";
    case SUSPENDED_STABLE:
      return "空中姿态控制，准备软着陆";
    case SUSPENDED_LONG_TERM:
      return "降低功耗，尝试恢复地面接触";
    case SUSPENDED_PERMANENT:
      return "紧急停止，等待人工干预";
    default:
      return "检查系统状态";
  }
}
