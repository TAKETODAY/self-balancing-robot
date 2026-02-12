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

#include "attitude_sensor.h"

#include <math.h>
#include "logging.hpp"
#include "esp/misc.hpp"
// #include "esp/platform.hpp"
// #include "esp/serial.hpp"

#define I2C_MASTER_NUM I2C_NUM_1          /*!< I2C port number for master dev */

#define RAD_TO_DEG     57.27272727f /*!< Radians to degrees */

static const char* TAG = "mpu6050";

static struct {
  mpu6050_axis_value_t acce;
  mpu6050_axis_value_t gyro;

  float roll;
  float pitch;
  float yaw;

  mpu6050_handle_t mpu6050;

  mpu6050_axis_value_t offset;

  const float accCoef; // Weight of gyroscope
  const float gyroCoef;

  float interval;
  uint64_t preInterval;

} this = {
  .accCoef = 0.02f,
  .gyroCoef = 0.98f,

};


void attitude_destory() {
  if (this.mpu6050) {
    mpu6050_delete(this.mpu6050);
  }
}

void attitude_begin() {
  this.mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
  ESP_ERROR_CHECK(mpu6050_config(this.mpu6050, ACCE_FS_2G, GYRO_FS_500DPS));
  ESP_ERROR_CHECK(mpu6050_wake_up(this.mpu6050));

  uint8_t mpu6050_deviceid;
  mpu6050_get_deviceid(this.mpu6050, &mpu6050_deviceid);
  log_debug("device-id: %d", mpu6050_deviceid);

  this.preInterval = millis();
  // calcGyroOffsets(true);
}

void attitude_set_gyro_offsets(float x, float y, float z) {
  this.offset.x = x;
  this.offset.y = y;
  this.offset.z = z;
}

void attitude_calc_gyro_offsets(bool console, uint16_t delayBefore, uint16_t delayAfter) {
  delay(delayBefore);
  if (console) {
    log_info("========================================\nCalculating gyro offsets\nDO NOT MOVE MPU6050\n", "");
  }

  float x = 0, y = 0, z = 0;

  for (int i = 0; i < 3000; i++) {
    mpu6050_get_gyro(this.mpu6050, &this.gyro);

    x += this.gyro.x;
    y += this.gyro.y;
    z += this.gyro.z;
  }

  this.offset.x = x / 3000;
  this.offset.y = y / 3000;
  this.offset.z = z / 3000;

  if (console) {
    log_info("Done! X : %5.f, Y : %5.f, Z : %5.f", this.offset.x, this.offset.y, this.offset.z);
    log_info("Program will start after %.2f seconds", delayAfter / 1000);
    delay(delayAfter);
  }
}

void attitude_update() {
  mpu6050_get_acce(this.mpu6050, &this.acce);
  mpu6050_get_gyro(this.mpu6050, &this.gyro);

  this.gyro.x -= this.offset.x;
  this.gyro.y -= this.offset.y;
  this.gyro.z -= this.offset.z;

  const float angleAccX = atan2f(this.acce.y, this.acce.z + fabsf(this.acce.x)) * RAD_TO_DEG;
  const float angleAccY = atan2f(this.acce.x, this.acce.z + fabsf(this.acce.y)) * -RAD_TO_DEG;

  this.interval = (float) (millis() - this.preInterval) * 0.001f;

  this.roll = this.gyroCoef * (this.roll + this.gyro.x * this.interval) + this.accCoef * angleAccX;
  this.pitch = this.gyroCoef * (this.pitch + this.gyro.y * this.interval) + this.accCoef * angleAccY;
  this.yaw = this.gyro.z * this.interval;

  this.preInterval = millis();
}

mpu6050_axis_value_t* attitude_get_gyroscope() {
  return &this.gyro;
}

mpu6050_axis_value_t* attitude_get_acceleration() {
  return &this.acce;
}

float attitude_get_pitch() {
  return this.pitch;
}

float attitude_get_roll() {
  return this.roll;
}

float attitude_get_yaw() {
  return this.yaw;
}
