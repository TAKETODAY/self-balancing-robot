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

#include "mpu6050.hpp"
#include "esp_log.h"
#include "esp/misc.hpp"

#define I2C_MASTER_SCL_IO gpio_num_t::GPIO_NUM_5     /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO gpio_num_t::GPIO_NUM_23     /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM i2c_port_t::I2C_NUM_1          /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 400000UL                   /*!< I2C master clock frequency */


static auto TAG = "mpu6050";

static mpu6050_handle_t mpu6050;

static void mpu6050Loop(void* pvParameters);

void mpu6050_init() {
  static i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_MASTER_SDA_IO;
  conf.scl_io_num = I2C_MASTER_SCL_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

  ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));

  mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);

  ESP_ERROR_CHECK(mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS));
  ESP_ERROR_CHECK(mpu6050_wake_up(mpu6050));

  xTaskCreate(mpu6050Loop, "mpu6050Loop", 2048, NULL, tskIDLE_PRIORITY, NULL);
}


static void mpu6050Loop(void* pvParameters) {
  ESP_LOGD(TAG, "mpu6050 looping");

  uint8_t mpu6050_deviceid;
  mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);

  ESP_LOGD(TAG, "device-id: %d", mpu6050_deviceid);

  mpu6050_acce_value_t acce;
  mpu6050_gyro_value_t gyro;
  mpu6050_temp_value_t temp;

  for (;;) {
    mpu6050_get_acce(mpu6050, &acce);
    ESP_LOGD(TAG, "acce_x:%.2f, acce_y:%.2f, acce_z:%.2f\n", acce.acce_x, acce.acce_y, acce.acce_z);

    mpu6050_get_gyro(mpu6050, &gyro);
    ESP_LOGD(TAG, "gyro_x:%.2f, gyro_y:%.2f, gyro_z:%.2f\n", gyro.gyro_x, gyro.gyro_y, gyro.gyro_z);

    mpu6050_get_temp(mpu6050, &temp);
    ESP_LOGD(TAG, "t:%.2f \n", temp.temp);

    delay(1000);
  }
}


void clean() {
  mpu6050_delete(mpu6050);
  i2c_driver_delete(I2C_MASTER_NUM);
}
