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

//
// typedef enum {
//   RISING = 0, /*!< Pin interrupt trigger mode: rising */
//   FALLING, /*!< Pin interrupt trigger mode: falling */
//   CHANGE, /*!< Pin interrupt trigger mode: rising or falling */
// } PinInterruptType;


#if (CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3)
#define NUM_OUPUT_PINS  46
#define PIN_DAC1        17
#define PIN_DAC2        18
#else
#define NUM_OUPUT_PINS  34
#define PIN_DAC1        25
#define PIN_DAC2        26
#endif

#define LOW               0x0
#define HIGH              0x1

//GPIO FUNCTIONS
#define INPUT             0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT)
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT            0x03
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x13
#define ANALOG            0xC0

//Interrupt Modes
#define DISABLED  0x00
#define RISING    0x01
#define FALLING   0x02
#define CHANGE    0x03
#define ONLOW     0x04
#define ONHIGH    0x05
#define ONLOW_WE  0x0C
#define ONHIGH_WE 0x0D


#define digitalPinIsValid(pin)          GPIO_IS_VALID_GPIO(pin)
#define digitalPinCanOutput(pin)        GPIO_IS_VALID_OUTPUT_GPIO(pin)

#define digitalPinToRtcPin(pin)         ((RTC_GPIO_IS_VALID_GPIO(pin))?rtc_io_number_get(pin):-1)
#define digitalPinToDacChannel(pin)     (((pin) == DAC_CHANNEL_1_GPIO_NUM)?0:((pin) == DAC_CHANNEL_2_GPIO_NUM)?1:-1)

/**
 * @brief Set GPIO Mode. Arduino style function.
 *
 * @param pin gpio pin number
 * @param mode gpio pin mode
 */
void pinMode(int pin, uint8_t mode);

/**
 * @brief Write GPIO Value. Arduino style function.
 *
 * @param pin gpio pin number
 * @param val gpio pin level
 */
void digitalWrite(int pin, uint8_t val);

/**
 * @brief Read GPIO Value. Arduino style function.
 *
 * @param pin gpio pin number
 * @return
 *     - 0 Low level
 *     - 1 High level
 */
int digitalRead(int pin);

/**
 * @brief Set GPIO Interrupt. Arduino style function.
 *
 * @param pin gpio pin number
 * @return uint8_t gpio pin number
 * @note
 *      This function only returns the pin number that needs to be interrupted by binding.
 *
 */
uint8_t digitalPinToInterrupt(int pin);

/**
 * @brief Bind function to GPIO Interrupt. Arduino style function.
 *
 * @param pin gpio pin number
 * @param handler interrupt function
 * @param mode interrupt triggering mode
 */
void attachInterrupt(int pin, gpio_isr_t handler, int mode);

// void attachInterruptArg(uint8_t pin, void (*)(void*), void* arg, int mode);
// void detachInterrupt(uint8_t pin);

int8_t digitalPinToTouchChannel(uint8_t pin);
int8_t digitalPinToAnalogChannel(uint8_t pin);
int8_t analogChannelToDigitalPin(uint8_t channel);


#ifdef __cplusplus
}
#endif
