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

#ifndef MAGNETICSENSORI2C_LIB_H
#define MAGNETICSENSORI2C_LIB_H

#include "esp/platform.hpp"
#include "i2c.hpp"
#include "../common/base_classes/Sensor.h"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"

struct MagneticSensorI2CConfig_s {
  uint8_t chip_address;
  uint8_t bit_resolution;
  uint8_t angle_register;
  uint8_t msb_mask;
  uint8_t msb_shift;
  uint8_t lsb_mask;
  uint8_t lsb_shift;
};

// some predefined structures
extern MagneticSensorI2CConfig_s AS5600_I2C, AS5048_I2C, MT6701_I2C;

#if defined(TARGET_RP2040)
#define SDA I2C_SDA
#define SCL I2C_SCL
#endif


class MagneticSensorI2C : public Sensor {
public:
  /**
   * MagneticSensorI2C class constructor
   * @param chip_address  I2C chip address
   * @param bit_resolution number of bits of the sensor resolution
   * @param angle_register_msb  angle read register msb
   * @param msb_bits_used number of used bits in msb
   */
  MagneticSensorI2C(uint8_t chip_address, int bit_resolution, uint8_t angle_register_msb, int msb_bits_used, bool lsb_right_aligned = true);

  /**
   * MagneticSensorI2C class constructor
   * @param config  I2C config
   */
  explicit MagneticSensorI2C(MagneticSensorI2CConfig_s config);

  /** sensor initialise pins */
  void init(espp::I2c* _wire);

  // implementation of abstract functions of the Sensor class
  /** get current angle (rad) */
  float getSensorAngle() override;

  /** experimental function to check and fix SDA locked LOW issues */
  int checkBus(byte sda_pin, byte scl_pin);

  /** current error code from Wire endTransmission() call **/
  uint8_t currWireError = 0;

private:
  float cpr; //!< Maximum range of the magnetic sensor
  MagneticSensorI2CConfig_s _conf;

  // I2C functions
  /** Read one I2C register value */
  int read(uint8_t angle_register_msb);

  /**
   * Function getting current angle register value
   * it uses angle_register variable
   */
  int getRawCount();

  /* the two wire instance for this sensor */
  espp::I2c* wire;
};


#endif
