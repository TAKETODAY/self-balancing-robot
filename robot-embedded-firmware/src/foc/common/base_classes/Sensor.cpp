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

#include "Sensor.h"
#include "../foc_utils.h"
#include "../time_utils.h"


void Sensor::update() {
  float val = getSensorAngle();
  if (val < 0) // sensor angles are strictly non-negative. Negative values are used to signal errors.
    return;    // TODO signal error, e.g. via a flag and counter
  angle_prev_ts = _micros();
  float d_angle = val - angle_prev;
  // if overflow happened track it as full rotation
  if (abs(d_angle) > (0.8f * _2PI)) full_rotations += (d_angle > 0) ? -1 : 1;
  angle_prev = val;
}


/** get current angular velocity (rad/s) */
float Sensor::getVelocity() {
  // calculate sample time
  float Ts = static_cast<float>(angle_prev_ts - vel_angle_prev_ts) * 1e-6f;
  if (Ts < 0.0f) {
    // handle micros() overflow - we need to reset vel_angle_prev_ts
    vel_angle_prev = angle_prev;
    vel_full_rotations = full_rotations;
    vel_angle_prev_ts = angle_prev_ts;
    return velocity;
  }
  if (Ts < min_elapsed_time) return velocity; // don't update velocity if deltaT is too small

  velocity = (static_cast<float>(full_rotations - vel_full_rotations) * _2PI + (angle_prev - vel_angle_prev)) / Ts;
  vel_angle_prev = angle_prev;
  vel_full_rotations = full_rotations;
  vel_angle_prev_ts = angle_prev_ts;
  return velocity;
}


void Sensor::init() {
  // initialize all the internal variables of Sensor to ensure a "smooth" startup (without a 'jump' from zero)
  getSensorAngle(); // call once
  delay_microseconds(1);
  vel_angle_prev = getSensorAngle(); // call again
  vel_angle_prev_ts = _micros();
  delay(1);
  getSensorAngle(); // call once
  delay_microseconds(1);
  angle_prev = getSensorAngle(); // call again
  angle_prev_ts = _micros();
}


float Sensor::getMechanicalAngle() {
  return angle_prev;
}


float Sensor::getAngle() {
  return (float) full_rotations * _2PI + angle_prev;
}

double Sensor::getPreciseAngle() {
  return (double) full_rotations * (double) _2PI + (double) angle_prev;
}

int32_t Sensor::getFullRotations() {
  return full_rotations;
}


int Sensor::needsSearch() {
  return 0; // default false
}
