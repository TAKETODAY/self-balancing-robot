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

#ifndef PID_H
#define PID_H


#include "time_utils.h"
#include "foc_utils.h"

/**
 *  PID controller class
 */
class PIDController {
public:
  /**
     *  
     * @param P - Proportional gain 
     * @param I - Integral gain
     * @param D - Derivative gain 
     * @param ramp - Maximum speed of change of the output value
     * @param limit - Maximum output value
     */
  PIDController(float P, float I, float D, float ramp, float limit);
  ~PIDController() = default;

  float operator()(float error);
  void reset();

  float P; //!< Proportional gain 
  float I; //!< Integral gain 
  float D; //!< Derivative gain 
  float output_ramp; //!< Maximum speed of change of the output value
  float limit; //!< Maximum output value

public:
  float error_prev; //!< last tracking error value
  float output_prev; //!< last pid output value
  float integral_prev; //!< last integral component value
  unsigned long timestamp_prev; //!< Last execution timestamp
};

#endif // PID_H