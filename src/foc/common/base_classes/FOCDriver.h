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

#ifndef FOCDRIVER_H
#define FOCDRIVER_H

#include "esp/platform.hpp"


enum PhaseState : uint8_t {
  PHASE_OFF = 0, // both sides of the phase are off
  PHASE_ON = 1,  // both sides of the phase are driven with PWM, dead time is applied in 6-PWM mode
  PHASE_HI = 2,  // only the high side of the phase is driven with PWM (6-PWM mode only)
  PHASE_LO = 3,  // only the low side of the phase is driven with PWM (6-PWM mode only)
};


enum DriverType{
    UnknownDriver=0,
    BLDC=1,
    Stepper=2,
    Hybrid=3
};

/**
 * FOC driver class
 */
class FOCDriver{
    public:

        /** Initialise hardware */
        virtual int init() = 0;
        /** Enable hardware */
        virtual void enable() = 0;
        /** Disable hardware */
        virtual void disable() = 0;

        long pwm_frequency; //!< pwm frequency value in hertz
        float voltage_power_supply; //!< power supply voltage
        float voltage_limit; //!< limiting voltage set to the motor

        bool initialized = false; //!< true if driver was successfully initialized
        void* params = 0; //!< pointer to hardware specific parameters of driver

        bool enable_active_high = true; //!< enable pin should be set to high to enable the driver (default is HIGH)

        /** get the driver type*/
        virtual DriverType type() = 0;
};

#endif
