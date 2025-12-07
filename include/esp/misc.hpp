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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get time in ms since boot.
 *
 * @return number of microseconds since underlying timer has been started
 */
unsigned long micros();

/**
 * @brief Get time in us since boot.
 *
 * @return number of milliseconds since underlying timer has been started
 */
unsigned long millis();

/**
 * @brief Delay us.
 *
 * @param us microsecond
 */
void delayMicroseconds(uint32_t us);

/**
 * @brief Rtos ms delay function
 *
 * @param ms millisecond
 */
void delay(uint32_t ms);

/**
 * @brief Minimum function.
 *
 * @param a numbers that need to be compared
 * @param b numbers that need to be compared
 * @return minimum value
 */
float min(float a, float b);

float max(float a, float b);

#ifdef __cplusplus
}
#endif
