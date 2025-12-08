// Copyright 2025 the original author or authors.
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

#include "robot.hpp"

#include "foc/sensors/MagneticSensorI2C.h"
#include "battery.hpp"
#include "esp_log.h"
#include "AttitudeSensor.hpp"
#include "nvs_flash.h"
#include "servos.hpp"

#include "esp/serial.hpp"
#include "wifi.h"

// Wrobot wrobot;

static auto TAG = "robot";

static BLDCMotor motor1(7);
static BLDCMotor motor2(7);

BLDCDriver3PWM driver1(32, 33, 25, 22);
BLDCDriver3PWM driver2(26, 27, 14, 12);

MagneticSensorI2C sensor1(AS5600_I2C);
MagneticSensorI2C sensor2(AS5600_I2C);


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


void nvs_init() {
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void robot_init() {
  nvs_init();

  serial.begin(115200);
  servos_init();

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
      ESP_LOGI(TAG, "Found devices at address: %d", address);
    }
  }

  sensor1.init(&i2c);
  sensor2.init(&i2c1);

  motor1.linkSensor(&sensor1);
  motor2.linkSensor(&sensor2);

  // 速度环PID参数
  motor1.PID_velocity.P = 0.05;
  motor1.PID_velocity.I = 1;
  motor1.PID_velocity.D = 0;

  motor2.PID_velocity.P = 0.05;
  motor2.PID_velocity.I = 1;
  motor2.PID_velocity.D = 0;

  // 驱动器设置
  motor1.voltage_sensor_align = 6;
  motor2.voltage_sensor_align = 6;
  driver1.voltage_power_supply = 8;
  driver2.voltage_power_supply = 8;
  driver1.init();
  driver2.init();

  // 连接motor对象与驱动器对象
  motor1.linkDriver(&driver1);
  motor2.linkDriver(&driver2);

  motor1.torque_controller = TorqueControlType::voltage; // 扭矩控制器类型为 "电压模式"
  motor2.torque_controller = TorqueControlType::voltage;
  motor1.controller = MotionControlType::torque; // 运动控制器类型为 "扭矩模式"
  motor2.controller = MotionControlType::torque;

  // monitor 相关设置
  motor1.useMonitoring(serial);
  motor2.useMonitoring(serial);

  // 电机初始化
  motor1.init();
  motor1.initFOC();
  motor2.init();
  motor2.initFOC();

  wifi_init();

  attitude.begin();
  attitude.calcGyroOffsets(true);

  battery_init();
}

RobotProtocol::RobotProtocol(uint8_t len) {
  _len = len;
  _now_buf = new uint8_t[_len];
  _old_buf = new uint8_t[_len];

  for (int i = 0; i < _len; i++) {
    _now_buf[i] = 0;
  }

  _now_buf[0] = 0xAA;
  _now_buf[1] = 0x55;
}

RobotProtocol::~RobotProtocol() {
  delete[] _now_buf;
  delete[] _old_buf;
}

void RobotProtocol::spinOnce() {
  int flag = checkBufRefresh();
  if (flag) {
    // UART_WriteBuf(); //这个会将web端的控制信息转成串口协议发出

    // Serial.println(wrobot.dir);//测试数据获取
    // Serial.println("date have send\n");
    // Serial.printf("height:%d\n", wrobot.height);
    // Serial.printf("roll:%d\n", wrobot.pitch);
    // Serial.printf("linear:%d\n", wrobot.linear);
    //        Serial.printf("\n");
    //        Serial.printf("joy_X:%d\n", wrobot.joyx);
    //        Serial.printf("joy_Y:%d\n", wrobot.joyy);
  }
}

/**************如下同时用于串口输出控制协议***************/

void RobotProtocol::UART_WriteBuf(void) {
  for (int i = 0; i < _len; i++) {
    // Serial.write(_now_buf[i]);
  }
}

int RobotProtocol::checkBufRefresh(void) {
  int ret = 0;
  for (int i = 0; i < _len; i++) {
    if (_now_buf[i] != _old_buf[i]) {
      ret = 1;
      break;
    }
    else {
      ret = 0;
    }
  }

  for (int i = 0; i < _len; i++) {
    _old_buf[i] = _now_buf[i];
  }
  return ret;
}

void RobotProtocol::parseBasic(StaticJsonDocument<300>& doc) {

  _now_buf[2] = BASIC;

  std::string dir = doc["dir"];
  if (dir == "stop") {
    _now_buf[3] = STOP;
    wrobot.dir = STOP;
  }
  else {
    if (dir == "jump") {
      _now_buf[3] = JUMP;
      wrobot.dir = JUMP;
    }
    else if (dir == "forward") {
      _now_buf[3] = FORWARD;
      wrobot.dir = FORWARD;
    }
    else if (dir == "back") {
      _now_buf[3] = BACK;
      wrobot.dir = BACK;
    }
    else if (dir == "left") {
      _now_buf[3] = LEFT;
      wrobot.dir = LEFT;
    }
    else if (dir == "right") {
      _now_buf[3] = RIGHT;
      wrobot.dir = RIGHT;
    }
    else {
      _now_buf[3] = STOP;
      wrobot.dir = STOP;
    }
  }

  int height = doc["height"];
  _now_buf[4] = height;
  wrobot.height = height;

  int roll = doc["roll"];
  wrobot.roll = roll;
  if (roll >= 0) {
    _now_buf[5] = 0;
  }
  else {
    _now_buf[5] = 1;
  }
  _now_buf[6] = abs(roll);

  int linear = doc["linear"];
  wrobot.linear = linear;
  if (linear >= 0) {
    _now_buf[7] = 0;
  }
  else {
    _now_buf[7] = 1;
  }
  _now_buf[8] = abs(linear);

  int angular = doc["angular"];
  wrobot.angular = angular;
  if (angular >= 0) {
    _now_buf[9] = 0;
  }
  else {
    _now_buf[9] = 1;
  }
  _now_buf[10] = abs(angular);

  int stable = doc["stable"];
  wrobot.go = stable;
  if (stable) {
    _now_buf[11] = 1;
  }
  else {
    _now_buf[11] = 0;
  }

  int joy_x = doc["joy_x"];
  wrobot.joyx = joy_x;
  if (joy_x >= 0) {
    _now_buf[12] = 0;
  }
  else {
    _now_buf[12] = 1;
  }
  _now_buf[13] = abs(joy_x);

  int joy_y = doc["joy_y"];
  wrobot.joyy = joy_y;
  if (joy_y >= 0) {
    _now_buf[14] = 0;
  }
  else {
    _now_buf[14] = 1;
  }
  _now_buf[15] = abs(joy_y);
}


// 舵机运动参数
byte ID[2] = { 1, 2 };
short Position[2];
unsigned short Speed[2];
byte ACC[2];

// LQR自平衡控制器参数
float LQR_angle = 0;
float LQR_gyro = 0;
float LQR_gyroZ = 0;
float LQR_speed = 0;
float LQR_distance = 0;
float angle_control = 0;
float gyro_control = 0;
float speed_control = 0;
float distance_control = 0;
float LQR_u = 0;
float pitch_zeropoint = 2; // 7 设定默认俯仰角度，向前到正，向后倒负
float original_pitch_zeropoint = pitch_zeropoint; // 保存原始的角度零点
float distance_zeropoint = 0.5f; // 轮部位移零点偏置
float pitch_adjust = 0.0f; // 俯仰角度调整,负数前倾，正数后倾


// 超级平衡模式参数
bool super_balance_mode = false; // 超级平衡模式
PIDController pid_super_balance(11, 0, 0, 100000, 100); // 适配joyy实际需求

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// lqr自平衡控制
// void lqr_balance_loop() {
//   /*
//   motor.target 负数小车向后，正数小车向前
//   LQR_u 越大，电机输出的转矩越大，电机向后转，默认0-8V
//   LQR_u = angle_control + gyro_control + distance_control + speed_control
//           = 俯仰角度控制 + 俯仰角速度控制 + 距离控制 + 速度控制
//
//     angle_control = pid_pitch(  LQR_angle       -   pitch_zeropoint) // 实际pitch角度-目标pitch角度
//                                 LQR_angle = mpu6050.getAngleY()
//
//     gyro_control = pid_gyro(  LQR_gyro - 0   ) // 实际角速度，目标值是零
//                               LQR_gyro =  mpu6050.getGyroY()
//
//     distance_control = pid_distance(  LQR_distance - distance_zeropoint) // 两个电机平均旋转弧度（实际位移量） - 位移基准零点（某一时刻LQR_distance快照）
//                                       LQR_distance = (-0.5)*(motor1.shaft_angle + motor2.shaft_angle)
//
//     speed_control = pid_speed(LQR_speed - speed_target_coeff * lpf_joyy(wrobot.joyy)) // 两个电机平均速度（实际速度） - 摇杆前后输入值* 系数 ，就速度差
//                               LQR_speed = (-0.5) * (motor1.shaft_velocity + motor2.shaft_velocity)
//                               speed_target_coeff = 前进后退系数
//   yaw 偏航角控制
//   float yaw_angle_control = pid_yaw_angle(yaw_target);
//                                           yaw_target = wrobot.joyx * 0.1; // 使用摇杆左右转向值
//
//   float yaw_gyro_control = pid_yaw_gyro(YAW_gyro); // yaw 轴的角速度控制
//                                         YAW_gyro =  (float)mpu6050.getGyroZ();
//
//   YAW_output = yaw_angle_control + yaw_gyro_control;
//
//   motor1.target = (-0.5) * (LQR_u + YAW_output); // target正数小车向前，负数向后
//   motor2.target = (-0.5) * (LQR_u - YAW_output);
//
//    */
//
//   LQR_distance = (-0.5) * (motor1.shaft_angle + motor2.shaft_angle); // 两个电机的旋转角度（shaft_angle）,单位：弧度（rad）实际位移量
//   LQR_speed = (-0.5) * (motor1.shaft_velocity + motor2.shaft_velocity); // 两个电机角速度（shaft_velocity）,单位：弧度 / 秒（rad/s）
//   LQR_angle =  attitude.getAngleY(); // mpu6050 pitch 角度，单位：度（°）
//
//   LQR_gyro = attitude.getGyroY(); // pitch Y轴角速度,单位：度 / 秒（°/s）
//
//   angle_control = pid_pitch(LQR_angle - pitch_zeropoint) + pitch_adjust; //
//
//   gyro_control = pid_gyro(LQR_gyro);
//
//   // 前进后退跳跃的系数都不一样
//   float speed_target_coeff = 0.1;
//   if (jump_flag) {
//     // 跳跃
//     speed_target_coeff = 0.1;
//   }
//   else if (wrobot.joyy > 0) {
//     // 前进
//     speed_target_coeff = 0.18;
//   }
//   else if (wrobot.joyy < 0) {
//     // 后退
//     speed_target_coeff = 0.11;
//   }
//   else {
//     speed_target_coeff = 0.1;
//   }
//
//   // 超级平衡模式
//   if (super_balance_mode && !is_falling && robot_enabled && !sitting_down && !jump_flag) {
//     // 误差=实际角度 - 默认零点
//     float balance_joyy = pid_super_balance(LQR_angle - pitch_zeropoint);
//     balance_joyy = constrain(balance_joyy, -100.0f, 100.0f);
//     wrobot.joyy = balance_joyy;
//   }
//
//   speed_control = pid_speed(LQR_speed - speed_target_coeff * lpf_joyy(wrobot.joyy)); // 最大8v
//
//   // 检测轮子差速，判断轮子是否离地
//   unsigned long current_time = millis();
//   if (current_time - last_speed_record_time >= SPEED_RECORD_INTERVAL) {
//     robot_speed_diff = LQR_speed - last_lqr_speed;
//
//     if (robot_speed_diff > 18.0) // 轮子离地
//     {
//       // wheel_ground_flag = 0; // 轮子离地标记
//       //  Serial.println("TAKE OFF");
//       // Serial.print(wheel_ground_flag);
//       // Serial.print(",robot_speed_diff:");
//       // Serial.println(robot_speed_diff);
//       // Serial.print(",jump_flag:");
//       // Serial.println(jump_flag); // 100 左右
//     }
//     if (robot_speed_diff<-9.0) // 轮子着地
//     {
//       // wheel_ground_flag = 1; // 轮子着地标记
//       // Serial.println("LANDING");
//       // Serial.print(",robot_speed_diff:");
//       // Serial.println(robot_speed_diff);
//       // Serial.print(",jump_flag:");
//       // Serial.println(jump_flag); // 120左右
//       if (jump_flag) {
//         // 落地点作为新的位移零点
//         resetZeroPoint();
//       }
//       if (jump_flag == 0) {
//         XboxKeyVibration(); // 大减速运动状态手柄震动
//       }
//     }
//     last_lqr_speed = LQR_speed; // 每隔100ms更新一次历史转速
//     last_speed_record_time = current_time; // 更新记录时间
//   }
//
//   // 重置位移零点和积分情形
//   // 1.判断摇杆是否没有前后左右运动指令
//   if ((wrobot.joyx_last != 0 && wrobot.joyx == 0) || (wrobot.joyy_last != 0 && wrobot.joyy == 0)) {
//     resetZeroPoint();
//   }
//
//   // 2. 运动中实时重置位移零点和积分情形
//   if (abs(robot_speed_diff) > 10 || (abs(LQR_speed) > 15) // 这两种是启动后自平衡，速度较快
//       || wrobot.joyy != 0 || sitting_down) {
//     resetZeroPoint();
//   }
//
//   distance_control = pid_distance(LQR_distance - distance_zeropoint);
//
//   // 计算 LQR_u
//   // 必须是在跳跃时jump_flag > 0，LQR_u= 角度控制量 + 角速度控制量
//   if ((jump_flag > 0 and jump_flag < 120)) // jump_flag：100离地 120着地
//   {
//     LQR_u = angle_control + gyro_control; // 角度控制量 + 角速度控制，位移和速度控制分量被忽略
//   }
//   else // 当轮部未离地时，LQR_u：4个参数
//   {
//     // 当轮部未离地时，LQR_u =角度控制量+角速度控制量+位移控制量+速度控制量
//     LQR_u = angle_control + gyro_control + distance_control + speed_control;
//   }
//
//   // 小车没有控制的时候自稳定状态
//   // 控制量lqr_u<5V，前进后退控制量很小， 遥控器无信号输入joyy=0，轮部位移控制正常介入distance_control<4，不处于跳跃后的恢复时期jump_flag=0,以及不是坐下状态
//   if (abs(LQR_u) < 5 && wrobot.joyy == 0 && abs(distance_control) < 4 && (jump_flag == 0)) //  && !sitting_down
//   {
//     LQR_u = pid_lqr_u(LQR_u); // 小转矩非线性补偿
//     pitch_zeropoint -= pid_zeropoint(lpf_zeropoint(distance_control)); // 重心自适应
//   }
//   else {
//     pid_lqr_u.error_prev = 0; // 输出积分清零
//   }
//
//   // 平衡控制参数自适应
//   // 考虑 leg_height_base = 0
//   if (wrobot.height < 50) {
//     pid_speed.P = 0.7;
//   }
//   else if (wrobot.height < 64) {
//     pid_speed.P = 0.6;
//   }
//   else {
//     pid_speed.P = 0.5;
//   }
// }
