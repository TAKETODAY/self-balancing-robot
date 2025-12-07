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
#include "mpu6050.hpp"
#include "nvs_flash.h"
#include "servos.hpp"

#include "esp/serial.hpp"
#include "wifi.h"

// Wrobot wrobot;

static auto TAG = "robot";

static SerialPort serial(UART_NUM_0);

static BLDCMotor motor1(7);
static BLDCMotor motor2(7);

BLDCDriver3PWM driver1(32, 33, 25, 22);
BLDCDriver3PWM driver2(26, 27, 14, 12);

MagneticSensorI2C sensor1(AS5600_I2C);
MagneticSensorI2C sensor2(AS5600_I2C);

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

  mpu6050_init();
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
