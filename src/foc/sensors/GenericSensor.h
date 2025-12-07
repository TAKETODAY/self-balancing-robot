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

#ifndef GENERIC_SENSOR_LIB_H
#define GENERIC_SENSOR_LIB_H

#include "esp/platform.hpp"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/base_classes/Sensor.h"


class GenericSensor : public Sensor {
public:
  /**
  GenericSensor class constructor
   * @param readCallback pointer to the function reading the sensor angle
   * @param initCallback pointer to the function initialising the sensor
  */
  GenericSensor(float (*readCallback)() = nullptr, void (*initCallback)() = nullptr);

  float (*readCallback)() = nullptr; //!< function pointer to sensor reading
  void (*initCallback)() = nullptr; //!< function pointer to sensor initialisation

  void init() override;

  // Abstract functions of the Sensor class implementation
  /** get current angle (rad) */
  float getSensorAngle() override;

};


#endif
