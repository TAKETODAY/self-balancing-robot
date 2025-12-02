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

#include "platform.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  gpio_config_t conf; /*!< gpio pin configuration */
  gpio_num_t number; /*!< gpio pin number */
} IoRecord;

typedef enum {
  INPUT = 0, /*!< Pin mode: input mode */
  OUTPUT, /*!< Pin mode: output mode */
  INPUT_PULLUP, /*!< Pin mode: input mode, enable pullup */
  INPUT_PULLDOWN, /*!< Pin mode: input mode, enable pulldown */
} PinMode;

typedef enum {
  LOW = 0, /*!< Pin level: low level*/
  HIGH, /*!< Pin level: high level*/
} PinLevel;

typedef enum {
  RISING = 0, /*!< Pin interrupt trigger mode: rising */
  FALLING, /*!< Pin interrupt trigger mode: falling */
  CHANGE, /*!< Pin interrupt trigger mode: rising or falling */
} PinInterruptType;

/**
 * @brief Set GPIO Mode. Arduino style function.
 *
 * @param pin gpio pin number
 * @param mode gpio pin mode
 */
void pinMode(gpio_num_t pin, PinMode mode);

/**
 * @brief Write GPIO Value. Arduino style function.
 *
 * @param pin gpio pin number
 * @param val gpio pin level
 */
void digitalWrite(gpio_num_t pin, PinLevel val);

/**
 * @brief Read GPIO Value. Arduino style function.
 *
 * @param pin gpio pin number
 * @return
 *     - 0 Low level
 *     - 1 High level
 */
int digitalRead(gpio_num_t pin);

/**
 * @brief Set GPIO Interrupt. Arduino style function.
 *
 * @param pin gpio pin number
 * @return uint8_t gpio pin number
 * @note
 *      This function only returns the pin number that needs to be interrupted by binding.
 *
 */
uint8_t digitalPinToInterrupt(gpio_num_t pin);

/**
 * @brief Bind function to GPIO Interrupt. Arduino style function.
 *
 * @param pin gpio pin number
 * @param handler interrupt function
 * @param mode interrupt triggering mode
 */
void attachInterrupt(gpio_num_t pin, gpio_isr_t handler, int mode);

#ifdef __cplusplus
}
#endif
