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

#ifndef BLDCDRIVER_H
#define BLDCDRIVER_H

#include "esp/platform.hpp"
#include "FOCDriver.h"

class BLDCDriver : public FOCDriver {
public:
  float dc_a; //!< currently set duty cycle on phaseA
  float dc_b; //!< currently set duty cycle on phaseB
  float dc_c; //!< currently set duty cycle on phaseC

  /**
   * Set phase voltages to the hardware
   *
   * @param Ua - phase A voltage
   * @param Ub - phase B voltage
   * @param Uc - phase C voltage
  */
  virtual void setPwm(float Ua, float Ub, float Uc) = 0;

  /**
   * Set phase state, enable/disable
   *
   * @param sc - phase A state : active / disabled ( high impedance )
   * @param sb - phase B state : active / disabled ( high impedance )
   * @param sa - phase C state : active / disabled ( high impedance )
  */
  virtual void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) = 0;

  /** driver type getter function */
  virtual DriverType type() override { return DriverType::BLDC; };
};

#endif
