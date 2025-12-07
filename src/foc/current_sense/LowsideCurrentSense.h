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

#ifndef LOWSIDE_CS_LIB_H
#define LOWSIDE_CS_LIB_H

#include "esp/platform.hpp"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "../common/base_classes/CurrentSense.h"
#include "../common/base_classes/FOCMotor.h"
#include "../common/base_classes/StepperDriver.h"
#include "../common/base_classes/BLDCDriver.h"
#include "../common/lowpass_filter.h"
#include "hardware_api.h"


class LowsideCurrentSense : public CurrentSense {
public:
  /**
    LowsideCurrentSense class constructor
    @param shunt_resistor shunt resistor value
    @param gain current-sense op-amp gain
    @param phA A phase adc pin
    @param phB B phase adc pin
    @param phC C phase adc pin (optional)
  */
  LowsideCurrentSense(float shunt_resistor, float gain, int pinA, int pinB, int pinC = _NC);
  /**
    LowsideCurrentSense class constructor
    @param mVpA mV per Amp ratio
    @param phA A phase adc pin
    @param phB B phase adc pin
    @param phC C phase adc pin (optional)
  */
  LowsideCurrentSense(float mVpA, int pinA, int pinB, int pinC = _NC);

  // CurrentSense interface implementing functions
  int init() override;
  PhaseCurrent_s getPhaseCurrents() override;

private:
  // gain variables
  float shunt_resistor; //!< Shunt resistor value
  float amp_gain; //!< amp gain value
  float volts_to_amps_ratio; //!< Volts to amps ratio

  /**
   *  Function finding zero offsets of the ADC
   */
  void calibrateOffsets();

};

#endif
