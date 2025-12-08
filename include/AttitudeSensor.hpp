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

  float getGyroX() { return gyro.gyro_x; }
  float getGyroY() { return gyro.gyro_y; }
  float getGyroZ() { return gyro.gyro_z; }

  float getGyroXoffset() { return gyroXoffset; }
  float getGyroYoffset() { return gyroYoffset; }
  float getGyroZoffset() { return gyroZoffset; }

  float getAccAngleX() { return angleAccX; }
  float getAccAngleY() { return angleAccY; }

  float getGyroAngleX() { return angleGyroX; }
  float getGyroAngleY() { return angleGyroY; }
  float getGyroAngleZ() { return angleGyroZ; }

  float getAngleX() { return angleX; }
  float getAngleY() { return angleY; }
  float getAngleZ() { return angleZ; }

public:
  mpu6050_acce_value_t acce{};
  mpu6050_gyro_value_t gyro{};

private:
  mpu6050_handle_t mpu6050;

  float gyroXoffset{}, gyroYoffset{}, gyroZoffset{};

  float angleGyroX{}, angleGyroY{}, angleGyroZ{},
      angleAccX{}, angleAccY{}, angleAccZ{};

  float angleX{}, angleY{}, angleZ{};

  float accCoef{}, gyroCoef{};

  float interval{};
  uint64_t preInterval{};

};

extern AttitudeSensor attitude;
