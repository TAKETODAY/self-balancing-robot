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

#pragma once


#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "misc.hpp"
#include "serial.hpp"
#include "gpio.hpp"

#define NOP() asm volatile("nop")
#define PI 3.14159265358979f

typedef bool boolean;
typedef uint8_t byte;
typedef unsigned int word;

class __FlashStringHelper;
#define F(string_literal) (((string_literal)))
