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

#ifndef STEPPERDRIVER_H
#define STEPPERDRIVER_H

#include "esp/platform.hpp"
#include "FOCDriver.h"

class StepperDriver : public FOCDriver {
public:
  /** 
         * Set phase voltages to the hardware 
         * 
         * @param Ua phase A voltage
         * @param Ub phase B voltage
        */
  virtual void setPwm(float Ua, float Ub) = 0;

  /**
         * Set phase state, enable/disable
         *
         * @param sc - phase A state : active / disabled ( high impedance )
         * @param sb - phase B state : active / disabled ( high impedance )
        */
  virtual void setPhaseState(PhaseState sa, PhaseState sb) = 0;

  /** driver type getter function */
  virtual DriverType type() override { return DriverType::Stepper; } ;
};

#endif