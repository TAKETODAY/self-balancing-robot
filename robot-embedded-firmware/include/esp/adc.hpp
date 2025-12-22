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

#include <stdint.h>
#include <sdkconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*!< RTC */
#define SOC_ADC_MAX_BITWIDTH                    (12)
#define SOC_RTC_SLOW_CLOCK_SUPPORT_8MD256       (1)

// #include "esp32-hal.h"

typedef enum {
    ADC_0db,
    ADC_2_5db,
    ADC_6db,
    ADC_11db,
    ADC_ATTENDB_MAX
} adc_attenuation_t;

/*
 * Get ADC value for pin
 * */
uint16_t analogRead(uint8_t pin);

/*
 * Get ADC value in default resolution for pin
 * */
uint16_t analogReadRaw(uint8_t pin);

/*
 * Get MilliVolts value for pin
 * */
uint32_t analogReadMilliVolts(uint8_t pin);

/*
 * Set the resolution of analogRead return values. Default is 12 bits (range from 0 to 4096).
 * If between 9 and 12, it will equal the set hardware resolution, else value will be shifted.
 * Range is 1 - 16
 *
 * Note: compatibility with Arduino SAM
 */
void analogReadResolution(uint8_t bits);

/*
 * Set the divider for the ADC clock.
 * Default is 1
 * Range is 1 - 255
 * */
void analogSetClockDiv(uint8_t clockDiv);

/*
 * Set the attenuation for all channels
 * Default is 11db
 * */
void analogSetAttenuation(adc_attenuation_t attenuation);

/*
 * Set the attenuation for particular pin
 * Default is 11db
 * */
void analogSetPinAttenuation(uint8_t pin, adc_attenuation_t attenuation);

/*
 * Attach pin to ADC (will also clear any other analog mode that could be on)
 * */
bool adcAttachPin(uint8_t pin);

#if CONFIG_IDF_TARGET_ESP32
/*
 * Sets the sample bits and read resolution
 * Default is 12bit (0 - 4095)
 * Range is 9 - 12
 * */
void analogSetWidth(uint8_t bits);

/*
 * Set pin to use for ADC calibration if the esp is not already calibrated (25, 26 or 27)
 * */
void analogSetVRefPin(uint8_t pin);

/*
 * Get value for HALL sensor (without LNA)
 * connected to pins 36(SVP) and 39(SVN)
 * */
int hallRead();
#endif

#ifdef __cplusplus
}
#endif

