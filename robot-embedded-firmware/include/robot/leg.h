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

#include "defs.h"

// @formatter:off
#ifdef __cplusplus
extern "C" {
#endif
//@formatter:on

void robot_leg_init();

void robot_leg_set_acceleration(uint8_t acceleration);

void robot_leg_set_speed(int left_speed, int right_speed);

//
void robot_leg_set_height_percentage(uint8_t percentage);

void robot_leg_set_left_height_percentage(uint8_t percentage);

void robot_leg_set_right_height_percentage(uint8_t percentage);

uint8_t robot_leg_get_left_height_percentage();

uint8_t robot_leg_get_right_height_percentage();

uint8_t robot_leg_get_height_percentage();

#ifdef __cplusplus
}
#endif
