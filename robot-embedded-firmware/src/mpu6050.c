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

#include "esp_system.h"
#include "driver/i2c.h"
#include "mpu6050.h"

#define ALPHA                       0.99f        /*!< Weight of gyroscope */
#define RAD_TO_DEG                  57.27272727f /*!< Radians to degrees */

/* MPU6050 register */
#define MPU6050_GYRO_CONFIG         0x1Bu
#define MPU6050_ACCEL_CONFIG        0x1Cu
#define MPU6050_INTR_PIN_CFG         0x37u
#define MPU6050_INTR_ENABLE          0x38u
#define MPU6050_INTR_STATUS          0x3Au
#define MPU6050_ACCEL_XOUT_H        0x3Bu
#define MPU6050_GYRO_XOUT_H         0x43u
#define MPU6050_TEMP_XOUT_H         0x41u
#define MPU6050_PWR_MGMT_1          0x6Bu
#define MPU6050_WHO_AM_I            0x75u

const uint8_t MPU6050_DATA_RDY_INT_BIT = BIT0;
const uint8_t MPU6050_I2C_MASTER_INT_BIT = BIT3;
const uint8_t MPU6050_FIFO_OVERFLOW_INT_BIT = BIT4;
const uint8_t MPU6050_MOT_DETECT_INT_BIT = BIT6;
const uint8_t MPU6050_ALL_INTERRUPTS = MPU6050_DATA_RDY_INT_BIT | MPU6050_I2C_MASTER_INT_BIT | MPU6050_FIFO_OVERFLOW_INT_BIT | MPU6050_MOT_DETECT_INT_BIT;

typedef struct {
  i2c_port_t bus;
  uint16_t dev_addr;
} mpu6050_dev_t;

static esp_err_t mpu6050_write(mpu6050_handle_t sensor, const uint8_t reg_start_addr, const uint8_t* const data_buf, const uint8_t data_len) {
  mpu6050_dev_t* sens = sensor;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  esp_err_t ret = i2c_master_start(cmd);
  assert(ESP_OK == ret);
  ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_WRITE, true);
  assert(ESP_OK == ret);
  ret = i2c_master_write_byte(cmd, reg_start_addr, true);
  assert(ESP_OK == ret);
  ret = i2c_master_write(cmd, data_buf, data_len, true);
  assert(ESP_OK == ret);
  ret = i2c_master_stop(cmd);
  assert(ESP_OK == ret);
  ret = i2c_master_cmd_begin(sens->bus, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  return ret;
}

static esp_err_t mpu6050_read(mpu6050_handle_t sensor, const uint8_t reg_start_addr, uint8_t* const data_buf, const uint8_t data_len) {
  mpu6050_dev_t* sens = sensor;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  esp_err_t ret = i2c_master_start(cmd);
  assert(ESP_OK == ret);
  ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_WRITE, true);
  assert(ESP_OK == ret);
  ret = i2c_master_write_byte(cmd, reg_start_addr, true);
  assert(ESP_OK == ret);
  ret = i2c_master_start(cmd);
  assert(ESP_OK == ret);
  ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_READ, true);
  assert(ESP_OK == ret);
  ret = i2c_master_read(cmd, data_buf, data_len, I2C_MASTER_LAST_NACK);
  assert(ESP_OK == ret);
  ret = i2c_master_stop(cmd);
  assert(ESP_OK == ret);
  ret = i2c_master_cmd_begin(sens->bus, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  return ret;
}

mpu6050_handle_t mpu6050_create(i2c_port_t port, const uint16_t dev_addr) {
  mpu6050_dev_t* sensor = calloc(1, sizeof(mpu6050_dev_t));
  sensor->bus = port;
  sensor->dev_addr = dev_addr << 1;
  return sensor;
}

void mpu6050_delete(mpu6050_handle_t sensor) {
  mpu6050_dev_t* sens = sensor;
  free(sens);
}

esp_err_t mpu6050_get_deviceid(mpu6050_handle_t sensor, uint8_t* const deviceid) {
  return mpu6050_read(sensor, MPU6050_WHO_AM_I, deviceid, 1);
}

esp_err_t mpu6050_wake_up(mpu6050_handle_t sensor) {
  uint8_t tmp;
  esp_err_t ret = mpu6050_read(sensor, MPU6050_PWR_MGMT_1, &tmp, 1);
  if (ESP_OK != ret) {
    return ret;
  }
  tmp &= (~BIT6);
  ret = mpu6050_write(sensor, MPU6050_PWR_MGMT_1, &tmp, 1);
  return ret;
}

esp_err_t mpu6050_sleep(mpu6050_handle_t sensor) {
  uint8_t tmp;
  esp_err_t ret = mpu6050_read(sensor, MPU6050_PWR_MGMT_1, &tmp, 1);
  if (ESP_OK != ret) {
    return ret;
  }
  tmp |= BIT6;
  ret = mpu6050_write(sensor, MPU6050_PWR_MGMT_1, &tmp, 1);
  return ret;
}

esp_err_t mpu6050_config(mpu6050_handle_t sensor, const mpu6050_acce_fs_t acce_fs, const mpu6050_gyro_fs_t gyro_fs) {
  const uint8_t config_regs[2] = { gyro_fs << 3, acce_fs << 3 };
  return mpu6050_write(sensor, MPU6050_GYRO_CONFIG, config_regs, sizeof(config_regs));
}

esp_err_t mpu6050_get_acce_sensitivity(mpu6050_handle_t sensor, float* const acce_sensitivity) {
  uint8_t acce_fs;
  const esp_err_t ret = mpu6050_read(sensor, MPU6050_ACCEL_CONFIG, &acce_fs, 1);
  acce_fs = (acce_fs >> 3) & 0x03;
  switch (acce_fs) {
    case ACCE_FS_2G:
      *acce_sensitivity = 16384;
      break;

    case ACCE_FS_4G:
      *acce_sensitivity = 8192;
      break;

    case ACCE_FS_8G:
      *acce_sensitivity = 4096;
      break;

    case ACCE_FS_16G:
      *acce_sensitivity = 2048;
      break;

    default:
      break;
  }
  return ret;
}

esp_err_t mpu6050_get_gyro_sensitivity(mpu6050_handle_t sensor, float* const gyro_sensitivity) {
  uint8_t gyro_fs;
  const esp_err_t ret = mpu6050_read(sensor, MPU6050_GYRO_CONFIG, &gyro_fs, 1);
  gyro_fs = (gyro_fs >> 3) & 0x03;
  switch (gyro_fs) {
    case GYRO_FS_250DPS:
      *gyro_sensitivity = 131.f;
      break;

    case GYRO_FS_500DPS:
      *gyro_sensitivity = 65.5f;
      break;

    case GYRO_FS_1000DPS:
      *gyro_sensitivity = 32.8f;
      break;

    case GYRO_FS_2000DPS:
      *gyro_sensitivity = 16.4f;
      break;

    default:
      break;
  }
  return ret;
}


esp_err_t mpu6050_get_raw_acce(mpu6050_handle_t sensor, mpu6050_raw_axis_value_t* const raw_acce_value) {
  uint8_t data_rd[6];
  const esp_err_t ret = mpu6050_read(sensor, MPU6050_ACCEL_XOUT_H, data_rd, sizeof(data_rd));

  raw_acce_value->raw_x = (int16_t) ((data_rd[0] << 8) + data_rd[1]);
  raw_acce_value->raw_y = (int16_t) ((data_rd[2] << 8) + data_rd[3]);
  raw_acce_value->raw_z = (int16_t) ((data_rd[4] << 8) + data_rd[5]);
  return ret;
}

esp_err_t mpu6050_get_raw_gyro(mpu6050_handle_t sensor, mpu6050_raw_axis_value_t* const raw_gyro_value) {
  uint8_t data_rd[6];
  const esp_err_t ret = mpu6050_read(sensor, MPU6050_GYRO_XOUT_H, data_rd, sizeof(data_rd));

  raw_gyro_value->raw_x = (int16_t) ((data_rd[0] << 8) + data_rd[1]);
  raw_gyro_value->raw_y = (int16_t) ((data_rd[2] << 8) + data_rd[3]);
  raw_gyro_value->raw_z = (int16_t) ((data_rd[4] << 8) + data_rd[5]);

  return ret;
}

esp_err_t mpu6050_get_acce(mpu6050_handle_t sensor, mpu6050_axis_value_t* const acce_value) {
  float acce_sensitivity;
  esp_err_t ret = mpu6050_get_acce_sensitivity(sensor, &acce_sensitivity);
  if (ret != ESP_OK) {
    return ret;
  }

  mpu6050_raw_axis_value_t acce;
  ret = mpu6050_get_raw_acce(sensor, &acce);
  if (ret != ESP_OK) {
    return ret;
  }

  acce_value->x = (float) acce.raw_x / acce_sensitivity;
  acce_value->y = (float) acce.raw_y / acce_sensitivity;
  acce_value->z = (float) acce.raw_z / acce_sensitivity;
  return ESP_OK;
}

esp_err_t mpu6050_get_gyro(mpu6050_handle_t sensor, mpu6050_axis_value_t* const gyro_value) {
  float gyro_sensitivity;
  esp_err_t ret = mpu6050_get_gyro_sensitivity(sensor, &gyro_sensitivity);
  if (ret != ESP_OK) {
    return ret;
  }

  mpu6050_raw_axis_value_t gyro;
  ret = mpu6050_get_raw_gyro(sensor, &gyro);
  if (ret != ESP_OK) {
    return ret;
  }

  gyro_value->x = (float) gyro.raw_x / gyro_sensitivity;
  gyro_value->y = (float) gyro.raw_y / gyro_sensitivity;
  gyro_value->z = (float) gyro.raw_z / gyro_sensitivity;
  return ESP_OK;
}

esp_err_t mpu6050_get_temp(mpu6050_handle_t sensor, mpu6050_temp_value_t* const temp_value) {
  uint8_t data_rd[2];
  esp_err_t ret = mpu6050_read(sensor, MPU6050_TEMP_XOUT_H, data_rd, sizeof(data_rd));
  temp_value->temp = (int16_t) ((data_rd[0] << 8) | (data_rd[1])) / 340.00 + 36.53;
  return ret;
}
