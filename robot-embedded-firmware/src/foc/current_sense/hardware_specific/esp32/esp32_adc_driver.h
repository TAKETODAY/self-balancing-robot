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

#ifndef SIMPLEFOC_ESP32_HAL_ADC_DRIVER_H_
#define SIMPLEFOC_ESP32_HAL_ADC_DRIVER_H_

/**
 * Get ADC value for pin
 * @param pin - pin number
 * @return ADC value (0 - 4095)
 * */
uint16_t adcRead(uint8_t pin);

/**
 * Initialize ADC pin
 * @param pin - pin number
 * 
 * @return true if success
 *          false if pin is not an ADC pin
 */
bool adcInit(uint8_t pin);

#endif /* ESP32 */