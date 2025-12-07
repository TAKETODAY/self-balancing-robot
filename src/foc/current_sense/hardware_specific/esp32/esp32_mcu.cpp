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

#include "esp32_mcu.h"


/**
 *  Inline adc reading implementation 
*/
// function reading an ADC value and returning the read voltage
float _readADCVoltageInline(const int pinA, const void* cs_params) {
  uint32_t raw_adc = adcRead(pinA);
  return raw_adc * ((ESP32CurrentSenseParams*) cs_params)->adc_voltage_conv;
}

// function reading an ADC value and returning the read voltage
void* _configureADCInline(const void* driver_params, const int pinA, const int pinB, const int pinC) {

  ESP32CurrentSenseParams* params = new ESP32CurrentSenseParams{
    .pins = { pinA, pinB, pinC },
    .adc_voltage_conv = (_ADC_VOLTAGE) / (_ADC_RESOLUTION)
  };

  // initialize the ADC pins
  // fail if the pin is not an ADC pin
  for (int i = 0; i < 3; i++) {
    if (_isset(params->pins[i])) {
      pinMode(params->pins[i], ANALOG);
      if (!adcInit(params->pins[i])) {
        // SIMPLEFOC_ESP32_CS_DEBUG("ERROR: Failed to initialise ADC pin: "+String(params->pins[i]) + String(", maybe not an ADC pin?"));
        SIMPLEFOC_ESP32_CS_DEBUG("ERROR: Failed to initialise ADC pin: %d, maybe not an ADC pin?", params->pins[i]);

        return SIMPLEFOC_CURRENT_SENSE_INIT_FAILED;
      }
    }
  }

  return params;
}
