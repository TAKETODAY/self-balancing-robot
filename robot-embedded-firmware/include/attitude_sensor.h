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

#pragma once

#include "mpu6050.h"

#ifdef __cplusplus
extern "C" {



#endif


void attitude_begin();

void attitude_set_gyro_offsets(float x, float y, float z);

void attitude_calc_gyro_offsets(bool console, uint16_t delayBefore, uint16_t delayAfter);

void attitude_update();

mpu6050_axis_value_t* attitude_get_gyroscope();
mpu6050_axis_value_t* attitude_get_acceleration();

float attitude_get_pitch();
float attitude_get_roll();
float attitude_get_yaw();

void attitude_destory();

#ifdef __cplusplus
}
#endif
