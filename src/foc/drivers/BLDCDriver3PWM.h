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

#ifndef BLDCDriver3PWM_h
#define BLDCDriver3PWM_h

#include "../common/base_classes/BLDCDriver.h"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "hardware_api.h"

/**
 3 pwm bldc driver class
*/
class BLDCDriver3PWM : public BLDCDriver {
public:
  /**
    BLDCDriver class constructor
    @param phA A phase pwm pin
    @param phB B phase pwm pin
    @param phC C phase pwm pin
    @param en1 enable pin (optional input)
    @param en2 enable pin (optional input)
    @param en3 enable pin (optional input)
  */
  BLDCDriver3PWM(int phA, int phB, int phC, int en1 = NOT_SET, int en2 = NOT_SET, int en3 = NOT_SET);

  /**  Motor hardware init function */
  int init() override;
  /** Motor disable function */
  void disable() override;
  /** Motor enable function */
  void enable() override;

  // hardware variables
  int pwmA; //!< phase A pwm pin number
  int pwmB; //!< phase B pwm pin number
  int pwmC; //!< phase C pwm pin number
  int enableA_pin; //!< enable pin number
  int enableB_pin; //!< enable pin number
  int enableC_pin; //!< enable pin number

  /**
   * Set phase voltages to the hardware
   *
   * @param Ua - phase A voltage
   * @param Ub - phase B voltage
   * @param Uc - phase C voltage
  */
  void setPwm(float Ua, float Ub, float Uc) override;

  /**
   * Set phase voltages to the hardware
   * > Only possible is the driver has separate enable pins for all phases!
   *
   * @param sc - phase A state : active / disabled ( high impedance )
   * @param sb - phase B state : active / disabled ( high impedance )
   * @param sa - phase C state : active / disabled ( high impedance )
  */
  virtual void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) override;

private:
};


#endif