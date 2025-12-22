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

#ifndef ESP32_MCU_CURRENT_SENSING_H
#define ESP32_MCU_CURRENT_SENSING_H

#include "../../hardware_api.h"

#include "../../../drivers/hardware_api.h"
#include "esp32_adc_driver.h"
#include "esp_log.h"

// esp32 current sense parameters
typedef struct ESP32CurrentSenseParams {
  int pins[3];
  float adc_voltage_conv;
  int adc_buffer[3] = {};
  int buffer_index = 0;
  int no_adc_channels = 0;
} ESP32CurrentSenseParams;

// macros for debugging wuing the simplefoc debug system
#define SIMPLEFOC_ESP32_CS_DEBUG(str, ...) \
  ESP_LOGD("simplefoc", "ESP32-CS: " str, __VA_ARGS__);

#define CHECK_CS_ERR(func_call, message) \
  if ((func_call) != ESP_OK) { \
    SIMPLEFOC_ESP32_CS_DEBUG("ERROR - %s" , message); \
    return SIMPLEFOC_CURRENT_SENSE_INIT_FAILED; \
  }


#define _ADC_VOLTAGE 3.3f
#define _ADC_RESOLUTION 4095.0f


#endif