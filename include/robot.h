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

#pragma once

// #include <WiFi.h>
#include <ArduinoJson.h>

typedef struct
{
  int height = 38;
  int roll;
  int linear;
  int angular;
  int dir;
  int dir_last;
  int joyy;
  int joyy_last;
  int joyx;
  int joyx_last;
  bool go;
} Wrobot;

extern Wrobot wrobot;

// 机器人运动状态枚举
typedef enum
{
  FORWARD = 0,
  BACK,
  RIGHT,
  LEFT,
  STOP,
  JUMP,
} QR_State_t;

// 机器人模式枚举类型
typedef enum
{
  BASIC = 0,
} Robot_Mode_t;

class RobotProtocol
{
public:
  RobotProtocol(uint8_t len);
  ~RobotProtocol();
  void spinOnce(void);
  void parseBasic(StaticJsonDocument<300> &doc);

private:
  uint8_t *_now_buf;
  uint8_t *_old_buf;
  uint8_t _len;
  void UART_WriteBuf(void);
  int checkBufRefresh(void);
};
