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

#include "AttitudeSensor.hpp"

#include <cmath>
#include <iostream>

#include "esp_log.h"
#include "esp/misc.hpp"
#include "esp/serial.hpp"

#define I2C_MASTER_NUM i2c_port_t::I2C_NUM_1          /*!< I2C port number for master dev */

static auto TAG = "mpu6050";

AttitudeSensor attitude;

AttitudeSensor::AttitudeSensor() :
  mpu6050(mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS)) {

}

AttitudeSensor::~AttitudeSensor() {
  if (mpu6050) {
    mpu6050_delete(mpu6050);
  }
}

void AttitudeSensor::begin() {
  ESP_ERROR_CHECK(mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS));
  ESP_ERROR_CHECK(mpu6050_wake_up(mpu6050));

  uint8_t mpu6050_deviceid;
  mpu6050_get_deviceid(this->mpu6050, &mpu6050_deviceid);
  ESP_LOGD(TAG, "device-id: %d", mpu6050_deviceid);

  preInterval = millis();
}

void AttitudeSensor::setGyroOffsets(float x, float y, float z) {
  gyroXoffset = x;
  gyroYoffset = y;
  gyroZoffset = z;
}

void AttitudeSensor::calcGyroOffsets(bool console, uint16_t delayBefore, uint16_t delayAfter) {
  delay(delayBefore);
  if (console) {
    serial.println();
    serial.println("========================================");
    serial.println("Calculating gyro offsets");
    serial.print("DO NOT MOVE MPU6050");
  }

  for (int i = 0; i < 3000; i++) {
    if (console && i % 1000 == 0) {
      serial.print(".");
    }

    update();
  }
  gyroXoffset = acce.acce_x / 3000;
  gyroYoffset = acce.acce_y / 3000;
  gyroZoffset = acce.acce_z / 3000;

  if (console) {
    serial.println();
    serial.println("Done!");
    serial.print("X : ");
    serial.println(gyroXoffset);
    serial.print("Y : ");
    serial.println(gyroYoffset);
    serial.print("Z : ");
    serial.println(gyroZoffset);
    // serial.println("Program will start after 3 seconds");
    serial.print("Program will start after ");
    serial.print(static_cast<float>(delayAfter) / 1000);
    serial.println(" seconds");
    delay(delayAfter);
  }
}

void AttitudeSensor::update() {
  mpu6050_get_acce(mpu6050, &acce);
  mpu6050_get_gyro(mpu6050, &gyro);

  gyro.gyro_x -= gyroXoffset;
  gyro.gyro_y -= gyroYoffset;
  gyro.gyro_z -= gyroZoffset;

  interval = static_cast<float>(millis() - preInterval) * 0.001f;

  angleGyroX += gyro.gyro_x * interval;
  angleGyroY += gyro.gyro_y * interval;
  angleGyroZ += gyro.gyro_z * interval;

  angleX = gyroCoef * (angleX + gyro.gyro_x * interval) + accCoef * angleAccX;
  angleY = gyroCoef * (angleY + gyro.gyro_y * interval) + accCoef * angleAccY;
  angleZ = angleGyroZ;

  preInterval = millis();
}
