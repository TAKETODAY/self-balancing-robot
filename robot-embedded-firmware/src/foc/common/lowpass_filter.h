// Copyright 2025 - 2026 the original author or authors.
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

#ifndef LOWPASS_FILTER_H
#define LOWPASS_FILTER_H

#include "defs.h"

/**
 *  Low pass filter
 */
class LowPassFilter {
public:
  /**
     * @param Tf - Low pass filter time constant
     */
  explicit LowPassFilter(float Tf);
  ~LowPassFilter() = default;

  float operator()(float x);
  float Tf; //!< Low pass filter time constant

private:
  uint64_t timestamp_prev; //!< Last execution timestamp
  float y_prev;            //!< filtered value in previous execution step
};

#endif // LOWPASS_FILTER_H
