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

#ifndef GENERIC_CS_LIB_H
#define GENERIC_CS_LIB_H

#include "esp/platform.hpp"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "../common/base_classes/CurrentSense.h"
#include "../common/lowpass_filter.h"
#include "hardware_api.h"


class GenericCurrentSense : public CurrentSense {
public:
  /**
    GenericCurrentSense class constructor
  */
  GenericCurrentSense (PhaseCurrent_s(*readCallback)() =
  nullptr
  ,
  void (*initCallback)() = nullptr
  );

  // CurrentSense interface implementing functions
  int init() override;
  PhaseCurrent_s getPhaseCurrents() override;
  int driverAlign(float align_voltage, bool modulation_centered) override;


  PhaseCurrent_s (*readCallback)() = nullptr; //!< function pointer to sensor reading
  void (*initCallback)() = nullptr; //!< function pointer to sensor initialisation

private:
  /**
   *  Function finding zero offsets of the ADC
   */
  void calibrateOffsets();
  float offset_ia; //!< zero current A voltage value (center of the adc reading)
  float offset_ib; //!< zero current B voltage value (center of the adc reading)
  float offset_ic; //!< zero current C voltage value (center of the adc reading)

};

#endif