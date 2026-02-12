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

#include "lowpass_filter.h"

LowPassFilter::LowPassFilter(float time_constant)
  : Tf(time_constant)
  , y_prev(0.0f) {
  timestamp_prev = _micros();
}


float LowPassFilter::operator()(float x) {
  const unsigned long timestamp = _micros();
  float dt = (timestamp - timestamp_prev) * 1e-6f;

  if (dt < 0.0f) {
    dt = 1e-3f;
  }
  else if (dt > 0.3f) {
    y_prev = x;
    timestamp_prev = timestamp;
    return x;
  }

  const float alpha = Tf / (Tf + dt);
  const float y = alpha * y_prev + (1.0f - alpha) * x;
  y_prev = y;
  timestamp_prev = timestamp;
  return y;
}
