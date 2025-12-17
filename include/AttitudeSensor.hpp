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

#include "mpu6050.h"

class AttitudeSensor {
public:
  AttitudeSensor();
  ~AttitudeSensor();

  void begin();

  void setGyroOffsets(float x, float y, float z);

  void calcGyroOffsets(bool console = false, uint16_t delayBefore = 1000, uint16_t delayAfter = 500);
  void update();

  float getGyroX() { return gyro.x; }
  float getGyroY() { return gyro.y; }
  float getGyroZ() { return gyro.z; }

  float getGyroXoffset() { return gyroXoffset; }
  float getGyroYoffset() { return gyroYoffset; }
  float getGyroZoffset() { return gyroZoffset; }

  float getAngleX() { return roll; }
  float getAngleY() { return pitch; }
  float getAngleZ() { return yaw; }

public:
  mpu6050_axis_value_t acce{};
  mpu6050_axis_value_t gyro{};

  float roll{}, pitch{}, yaw{};

private:
  mpu6050_handle_t mpu6050;

  float gyroXoffset;
  float gyroYoffset;
  float gyroZoffset;

  float accCoef; // Weight of gyroscope
  float gyroCoef;

  float interval{};
  uint64_t preInterval{};

};

extern AttitudeSensor attitude;
