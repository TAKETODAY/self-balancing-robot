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

#include "lqr_controller.hpp"

#include "attitude_sensor.h"
#include "logging.hpp"
#include "robot.hpp"

#include "foc/BLDCMotor.h"
#include "foc/common/pid.h"
#include "foc/drivers/BLDCDriver3PWM.h"
#include "foc/sensors/MagneticSensorI2C.h"

#define balance_CORE 1

static auto TAG = "LQR-controller";

static BLDCMotor motor_L(7);
static BLDCMotor motor_R(7);

BLDCDriver3PWM driverL(32, 33, 25, 22);
BLDCDriver3PWM driverR(26, 27, 14, 12);

MagneticSensorI2C sensorL{ AS5600_I2C };
MagneticSensorI2C sensorR{ AS5600_I2C };


// PID控制器实例
PIDController pid_pitch(1, 0, 0, 100000, 8); // 比例系数（P）, 积分系数（I）, 微分系数（D）, 积分限幅(当I=0,无效), 输出限幅（8V防止电机过载）
PIDController pid_gyro(0.06, 0, 0, 100000, 8);
PIDController pid_distance(0.5, 0, 0, 100000, 8);
PIDController pid_speed(0.7, 0, 0, 100000, 8);
PIDController pid_yaw_angle(1.0, 0, 0, 100000, 8);
PIDController pid_yaw_gyro(0.04, 0, 0, 100000, 8);
PIDController pid_lqr_u(1, 15, 0, 100000, 8);
PIDController pid_zeropoint(0.002, 0, 0, 100000, 4);
PIDController pid_roll_angle(8, 0, 0, 100000, 450);

// 低通滤波器实例
LowPassFilter lpf_joyy(0.2); // 新输入值占输出值的10%，历史值占90%
LowPassFilter lpf_zeropoint(0.1);
LowPassFilter lpf_roll(0.3);
LowPassFilter lpf_height(0.1);

static constexpr float K_SCALE = -0.5f;

static void foc_balance_loop(void* pvParameters);

void robot_suspended_controller_init();

void lqr_controller::begin() {
  static espp::I2c i2c({
    .port = I2C_NUM_0,
    .sda_io_num = GPIO_NUM_19,
    .scl_io_num = GPIO_NUM_18,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .clk_speed = 400000UL
  });

  static espp::I2c i2c1({
    .port = I2C_NUM_1,
    .sda_io_num = GPIO_NUM_23,
    .scl_io_num = GPIO_NUM_5,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .clk_speed = 400000UL
  });

  for (uint8_t address = 1; address < 128; address++) {
    if (i2c1.probe_device(address)) {
      log_info("Found devices at address: %d", address);
    }
  }

  attitude_begin();

  sensorL.init(&i2c);
  sensorR.init(&i2c1);

  motor_L.linkSensor(&sensorL);
  motor_R.linkSensor(&sensorR);

  // 速度环PID参数
  motor_L.PID_velocity.P = 0.05;
  motor_L.PID_velocity.I = 1;
  motor_L.PID_velocity.D = 0;

  motor_R.PID_velocity.P = 0.05;
  motor_R.PID_velocity.I = 1;
  motor_R.PID_velocity.D = 0;

  // 驱动器设置
  motor_L.voltage_sensor_align = 6;
  motor_R.voltage_sensor_align = 6;
  driverL.voltage_power_supply = 8;
  driverR.voltage_power_supply = 8;
  driverL.init();
  driverR.init();

  // 连接motor对象与驱动器对象
  motor_L.linkDriver(&driverL);
  motor_R.linkDriver(&driverR);

  motor_L.torque_controller = TorqueControlType::voltage; // 扭矩控制器类型为 "电压模式"
  motor_R.torque_controller = TorqueControlType::voltage;
  motor_L.controller = MotionControlType::torque; // 运动控制器类型为 "扭矩模式"
  motor_R.controller = MotionControlType::torque;

  // monitor 相关设置
  motor_L.useMonitoring(serial);
  motor_R.useMonitoring(serial);

  // 电机初始化
  motor_L.init();
  motor_L.initFOC();
  motor_R.init();
  motor_R.initFOC();

  xTaskCreatePinnedToCore(foc_balance_loop, "balance_loop", 4096, this, 10, &task_handle, balance_CORE);
}

static void stop_motors() {
  motor_L.target = 0;
  motor_R.target = 0;
}


static void foc_balance_loop(void* pvParameters) {
  auto* controller = static_cast<lqr_controller*>(pvParameters);
  log_info("foc balance looping");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  constexpr TickType_t xFrequency = pdMS_TO_TICKS(5);

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    attitude_update();

    controller->balance_loop();
    controller->yaw_loop();

    wrobot.joyx_last = wrobot.joyx;
    wrobot.joyy_last = wrobot.joyy;

    if (abs(controller->LQR_angle) > 40.0f) {
      stop_motors();
    }
    else {
      motor_L.target = K_SCALE * (controller->LQR_u + controller->YAW_output);
      motor_R.target = K_SCALE * (controller->LQR_u - controller->YAW_output);
    }
    motor_L.loopFOC();
    motor_R.loopFOC();

    motor_L.move();
    motor_R.move();
  }
}

// 重置距离零点
void lqr_controller::resetZeroPoint() {
  distance_zeropoint = LQR_distance;
  pid_lqr_u.error_prev = 0;
  pitch_adjust = 0.0f;
}


// lqr自平衡控制
void lqr_controller::balance_loop() {
  LQR_distance = K_SCALE * (motor_L.shaft_angle + motor_R.shaft_angle);    // 两个电机的旋转角度（shaft_angle）,单位：弧度（rad）实际位移量
  LQR_speed = K_SCALE * (motor_L.shaft_velocity + motor_R.shaft_velocity); // 两个电机角速度（shaft_velocity）,单位：弧度 / 秒（rad/s）
  LQR_angle = attitude_get_pitch();                                        // mpu6050 pitch 角度，单位：度（°）

  LQR_gyro = attitude_get_gyroscope()->y; // pitch Y轴角速度,单位：度 / 秒（°/s）

  angle_control = pid_pitch(LQR_angle - pitch_zeropoint) + pitch_adjust;

  gyro_control = pid_gyro(LQR_gyro);

  // 前进后退跳跃的系数都不一样
  float speed_target_coeff = 0.1;
  if (wrobot.joyy > 0) {
    // 前进
    speed_target_coeff = 0.18;
  }
  else if (wrobot.joyy < 0) {
    // 后退
    speed_target_coeff = 0.11;
  }

  speed_control = pid_speed(LQR_speed - speed_target_coeff * lpf_joyy(wrobot.joyy)); // 最大8v

  // 检测轮子差速，判断轮子是否离地
  if (unsigned long current_time = millis(); current_time - last_speed_record_time >= SPEED_RECORD_INTERVAL) {
    robot_speed_diff = LQR_speed - last_lqr_speed;
    // 轮子离地
    if (robot_speed_diff > 18.0) {
    }
    if (robot_speed_diff < -9.0) {
      // 轮子着地
      if (jump_flag) {
        // 落地点作为新的位移零点
        resetZeroPoint();
      }
      if (jump_flag == 0) {
        // 大减速运动状态手柄震动
      }
    }
    last_lqr_speed = LQR_speed;            // 每隔100ms更新一次历史转速
    last_speed_record_time = current_time; // 更新记录时间
  }

  // 重置位移零点和积分情形
  // 1.判断摇杆是否没有前后左右运动指令
  if ((wrobot.joyx_last != 0 && wrobot.joyx == 0) || (wrobot.joyy_last != 0 && wrobot.joyy == 0)) {
    resetZeroPoint();
  }

  // 2. 运动中实时重置位移零点和积分情形
  if (abs(robot_speed_diff) > 10 || abs(LQR_speed) > 15 || wrobot.joyy != 0) {
    // 这两种是启动后自平衡，速度较快
    resetZeroPoint();
  }

  distance_control = pid_distance(LQR_distance - distance_zeropoint);

  // 计算 LQR_u
  // 必须是在跳跃时jump_flag > 0，LQR_u= 角度控制量 + 角速度控制量
  if (jump_flag > 0 and jump_flag < 120) {
    // jump_flag：100离地 120着地
    LQR_u = angle_control + gyro_control; // 角度控制量 + 角速度控制，位移和速度控制分量被忽略
  }
  else {
    // 当轮部未离地时，LQR_u：4个参数
    // 当轮部未离地时，LQR_u =角度控制量+角速度控制量+位移控制量+速度控制量
    LQR_u = angle_control + gyro_control + distance_control + speed_control;
  }

  // 小车没有控制的时候自稳定状态
  // 控制量lqr_u<5V，前进后退控制量很小， 遥控器无信号输入joyy=0，轮部位移控制正常介入distance_control<4，不处于跳跃后的恢复时期jump_flag=0,以及不是坐下状态
  if (abs(LQR_u) < 5 && wrobot.joyy == 0 && abs(distance_control) < 4 && jump_flag == 0) {
    LQR_u = pid_lqr_u(LQR_u);                                          // 小转矩非线性补偿
    pitch_zeropoint -= pid_zeropoint(lpf_zeropoint(distance_control)); // 重心自适应
  }
  else {
    pid_lqr_u.error_prev = 0; // 输出积分清零
  }

  // 平衡控制参数自适应
  // 考虑 leg_height_base = 0
  if (wrobot.height < 50) {
    pid_speed.P = 0.7;
  }
  else if (wrobot.height < 64) {
    pid_speed.P = 0.6;
  }
  else {
    pid_speed.P = 0.5;
  }

}

void lqr_controller::yaw_loop() {
  // 跳跃中，YAW_output 设为0，避免干扰左右旋转
  if (jump_flag) {
    YAW_output = 0;
    return;
  }

  // 1. 根据abs(wrobot.joyx)，动态设置yaw_target系数
  // float coeff = map100(abs(wrobot.joyx), 10, 100) / 100.0f;
  float coeff = mapf(abs(wrobot.joyx), 0, 100, 0.1f, 1.0f);

  coeff = constrain(coeff, 0.1f, 1.0f);
  float yaw_target = (float) wrobot.joyx * coeff;

  // 2. 根据abs(wrobot.joyx)，提高PID响应：临时放大yaw_angle_control（核心突破上限）
  float yaw_angle_control_coeff = 1.0f;
  if (wrobot.joyy == 0 && abs(wrobot.joyx) > 10) {
    // 原地打转
    yaw_angle_control_coeff = mapf(abs(wrobot.joyx), 0, 100, 1.0, 2.0);
    // 根据转向速度，智能计算wrobot.height
    // abs(wrobot.joyx) 范围0到100，默认0，输出范围20-50,默认 30
    // 反向映射：绝对值30→60，绝对值100→0（线性过渡）

    int height = mapi(abs(wrobot.joyx), 0, 100, 50, 30);
    // 强制约束输出在0~60范围内（防止异常值）
    height = constrain(height, 30, 50);
    // 使用低波过滤器，平滑数据
    wrobot.height = (int) lpf_height((float) height);

    // 增加rgb效果
    // startLEDBlink(CRGB::Red, 200, 1);
  }

  float yaw_angle_control = pid_yaw_angle(yaw_target) * yaw_angle_control_coeff;
  // 限制yaw_angle_control上限，避免超压（按电机7.4V反推，设为12较合适）
  yaw_angle_control = constrain(yaw_angle_control, -12.0f, 12.0f);

  YAW_gyro = attitude_get_gyroscope()->z; // 左右偏航角速度，用于纠正小车前后走直线时的角度偏差
  float yaw_gyro_control = pid_yaw_gyro(YAW_gyro);
  YAW_output = yaw_angle_control + yaw_gyro_control;
}

void lqr_controller::stop() {
  log_info("stop");
  stop_motors();

  motor_L.disable();
  motor_R.disable();

  if (const eTaskState state = eTaskGetState(task_handle); state != eSuspended) {
    vTaskSuspend(task_handle);
  }
}

void lqr_controller::start() {
  log_info("start");
  if (const eTaskState state = eTaskGetState(task_handle); state == eSuspended) {

    motor_L.enable();
    motor_R.enable();

    vTaskResume(task_handle);
  }

}

bool lqr_controller::is_started() {
  return eTaskGetState(task_handle) == eRunning;
}