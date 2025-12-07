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


#include "esp/adc.hpp"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "logging.hpp"
#include "esp/gpio.hpp"

#if SOC_DAC_SUPPORTED           //ESP32, ESP32S2
#include "soc/dac_channel.h"
#include "soc/sens_reg.h"
#include "soc/rtc_io_reg.h"
#endif

#define DEFAULT_VREF    1100

static uint8_t __analogAttenuation = 3; //11db
static uint8_t __analogWidth = ADC_WIDTH_MAX - 1; //3 for ESP32/ESP32C3; 4 for ESP32S2
static uint8_t __analogReturnedWidth = SOC_ADC_MAX_BITWIDTH; //12 for ESP32/ESP32C3; 13 for ESP32S2
static uint8_t __analogClockDiv = 1;
static adc_attenuation_t __pin_attenuation[SOC_GPIO_PIN_COUNT];

static uint16_t __analogVRef = 0;
#if CONFIG_IDF_TARGET_ESP32
static uint8_t __analogVRefPin = 0;
#endif

static const char* TAG = "ADC";

static inline uint16_t mapResolution(uint16_t value) {
  uint8_t from = __analogWidth + 9;
  if (from == __analogReturnedWidth) {
    return value;
  }
  if (from > __analogReturnedWidth) {
    return value >> (from - __analogReturnedWidth);
  }
  return value << (__analogReturnedWidth - from);
}

void __analogSetClockDiv(uint8_t clockDiv) {
  if (!clockDiv) {
    clockDiv = 1;
  }
  __analogClockDiv = clockDiv;
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
  adc_set_clk_div(__analogClockDiv);
#endif
}

void __analogSetAttenuation(adc_attenuation_t attenuation) {
  __analogAttenuation = attenuation & 3;
}

#if CONFIG_IDF_TARGET_ESP32
void __analogSetWidth(uint8_t bits) {
  if (bits < 9) {
    bits = 9;
  }
  else if (bits > 12) {
    bits = 12;
  }
  __analogWidth = bits - 9;
  adc1_config_width(static_cast<adc_bits_width_t>(__analogWidth));
}
#endif

void __analogInit() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  __analogSetClockDiv(__analogClockDiv);
#if CONFIG_IDF_TARGET_ESP32
  __analogSetWidth(__analogWidth + 9); //in bits
#endif
  for (int i = 0; i < SOC_GPIO_PIN_COUNT; i++) {
    __pin_attenuation[i] = ADC_ATTENDB_MAX;
  }
}

void __analogSetPinAttenuation(uint8_t pin, adc_attenuation_t attenuation) {
  int8_t channel = digitalPinToAnalogChannel(pin);
  if (channel < 0 || attenuation > 3) {
    return;
  }
  if (channel > (SOC_ADC_MAX_CHANNEL_NUM - 1)) {
    adc2_config_channel_atten(static_cast<adc2_channel_t>(channel - SOC_ADC_MAX_CHANNEL_NUM), static_cast<adc_atten_t>(attenuation));
  }
  else {
    adc1_config_channel_atten(static_cast<adc1_channel_t>(channel), static_cast<adc_atten_t>(attenuation));
  }
  __analogInit();
  if ((__pin_attenuation[pin] != ADC_ATTENDB_MAX) || (attenuation != __analogAttenuation)) {
    __pin_attenuation[pin] = attenuation;
  }
}

bool __adcAttachPin(uint8_t pin) {
  int8_t channel = digitalPinToAnalogChannel(pin);
  if (channel < 0) {
    log_error("Pin %u is not ADC pin!", pin);
    return false;
  }
  __analogInit();
  int8_t pad = digitalPinToTouchChannel(pin);
  if (pad >= 0) {
#if CONFIG_IDF_TARGET_ESP32
    uint32_t touch = READ_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG);
    if (touch & (1 << pad)) {
      touch &= ~((1 << (pad + SENS_TOUCH_PAD_OUTEN2_S))
                 | (1 << (pad + SENS_TOUCH_PAD_OUTEN1_S))
                 | (1 << (pad + SENS_TOUCH_PAD_WORKEN_S)));
      WRITE_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG, touch);
    }
#endif
  }
#if SOC_DAC_SUPPORTED
  else if (pin == DAC_CHANNEL_1_GPIO_NUM) {
    CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE); //stop dac1
  }
  else if (pin == DAC_CHANNEL_2_GPIO_NUM) {
    CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE); //stop dac2
  }
#endif

  pinMode(pin, ANALOG);
  __analogSetPinAttenuation(pin, static_cast<adc_attenuation_t>(__pin_attenuation[pin] != ADC_ATTENDB_MAX ? __pin_attenuation[pin] : __analogAttenuation));
  return true;
}

void __analogReadResolution(uint8_t bits) {
  if (!bits || bits > 16) {
    return;
  }
  __analogReturnedWidth = bits;
#if CONFIG_IDF_TARGET_ESP32
  __analogSetWidth(bits); // hadware from 9 to 12
#endif
}

uint16_t __analogReadRaw(uint8_t pin) {
  int8_t channel = digitalPinToAnalogChannel(pin);
  int value = 0;
  esp_err_t r = ESP_OK;
  if (channel < 0) {
    log_error("Pin %u is not ADC pin!", pin);
    return value;
  }
  __adcAttachPin(pin);
  if (channel > (SOC_ADC_MAX_CHANNEL_NUM - 1)) {
    channel -= SOC_ADC_MAX_CHANNEL_NUM;
    r = adc2_get_raw(static_cast<adc2_channel_t>(channel), static_cast<adc_bits_width_t>(__analogWidth), &value);
    if (r == ESP_OK) {
      return mapResolution(value);
    }
    else if (r == ESP_ERR_INVALID_STATE) {
      log_error("GPIO%u: %s: ADC2 not initialized yet.", pin, esp_err_to_name(r));
    }
    else if (r == ESP_ERR_TIMEOUT) {
      log_error("GPIO%u: %s: ADC2 is in use by Wi-Fi. Please see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-limitations for more info", pin,
        esp_err_to_name(r));
    }
    else {
      log_error("GPIO%u: %s", pin, esp_err_to_name(r));
    }
  }
  else {
    value = adc1_get_raw(static_cast<adc1_channel_t>(channel));
    return value;
  }
  return value;
}

uint16_t __analogRead(uint8_t pin) {
  uint16_t value = __analogReadRaw(pin);
  return mapResolution(value);
}

uint32_t __analogReadMilliVolts(uint8_t pin) {
  int8_t channel = digitalPinToAnalogChannel(pin);
  if (channel < 0) {
    log_error("Pin %u is not ADC pin!", pin);
    return 0;
  }

  if (!__analogVRef) {
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
      log_debug("eFuse Two Point: Supported");
      __analogVRef = DEFAULT_VREF;
    }
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
      log_debug("eFuse Vref: Supported");
      __analogVRef = DEFAULT_VREF;
    }
    if (!__analogVRef) {
      __analogVRef = DEFAULT_VREF;

#if CONFIG_IDF_TARGET_ESP32
      if (__analogVRefPin) {
        esp_adc_cal_characteristics_t chars;
        if (adc_vref_to_gpio(ADC_UNIT_2, static_cast<gpio_num_t>(__analogVRefPin)) == ESP_OK) {
          __analogVRef = __analogReadRaw(__analogVRefPin);
          esp_adc_cal_characterize(adc_unit_t::ADC_UNIT_1, static_cast<adc_atten_t>(__analogAttenuation), static_cast<adc_bits_width_t>(__analogWidth), DEFAULT_VREF, &chars);
          __analogVRef = esp_adc_cal_raw_to_voltage(__analogVRef, &chars);
          log_debug("Vref to GPIO%u: %u", __analogVRefPin, __analogVRef);
        }
      }
#endif
    }
  }
  uint8_t unit = 1;
  if (channel > (SOC_ADC_MAX_CHANNEL_NUM - 1)) {
    unit = 2;
  }

  uint16_t adc_reading = __analogReadRaw(pin);

  uint8_t atten = __analogAttenuation;
  if (__pin_attenuation[pin] != ADC_ATTENDB_MAX) {
    atten = __pin_attenuation[pin];
  }

  esp_adc_cal_characteristics_t chars = {};
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(static_cast<adc_unit_t>(unit),
    static_cast<adc_atten_t>(atten), static_cast<adc_bits_width_t>(__analogWidth), __analogVRef, &chars);

  static bool print_chars_info = true;
  if (print_chars_info) {
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      log_info("ADC%u: Characterized using Two Point Value: %u\n", unit, chars.vref);
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      log_info("ADC%u: Characterized using eFuse Vref: %u\n", unit, chars.vref);
    }
#if CONFIG_IDF_TARGET_ESP32
    else if (__analogVRef != DEFAULT_VREF) {
      log_info("ADC%u: Characterized using Vref to GPIO%u: %u\n", unit, __analogVRefPin, chars.vref);
    }
#endif
    else {
      log_info("ADC%u: Characterized using Default Vref: %u\n", unit, chars.vref);
    }
    print_chars_info = false;
  }
  return esp_adc_cal_raw_to_voltage((uint32_t) adc_reading, &chars);
}

#if CONFIG_IDF_TARGET_ESP32

void __analogSetVRefPin(uint8_t pin) {
  if (pin < 25 || pin > 27) {
    pin = 0;
  }
  __analogVRefPin = pin;
}

// int __hallRead() //hall sensor using idf read
// {
//   pinMode(36, ANALOG);
//   pinMode(39, ANALOG);
//   __analogSetWidth(12);
//   return hall_sensor_read();
// }
#endif

uint16_t analogRead(uint8_t pin) {
  return __analogRead(pin);
};

uint16_t analogReadRaw(uint8_t pin) {
  return __analogReadRaw(pin);
};

uint32_t analogReadMilliVolts(uint8_t pin) {
  return __analogReadMilliVolts(pin);
}

extern void analogReadResolution(uint8_t bits) {
  __analogReadResolution(bits);
}

extern void analogSetClockDiv(uint8_t clockDiv) {
  __analogSetClockDiv(clockDiv);
}

extern void analogSetAttenuation(adc_attenuation_t attenuation) {
  __analogSetAttenuation(attenuation);
};

extern void analogSetPinAttenuation(uint8_t pin, adc_attenuation_t attenuation) {
  __analogSetPinAttenuation(pin, attenuation);
}

extern bool adcAttachPin(uint8_t pin) {
  return __adcAttachPin(pin);
}

#if CONFIG_IDF_TARGET_ESP32
extern void analogSetVRefPin(uint8_t pin) {
  __analogSetVRefPin(pin);
}

extern void analogSetWidth(uint8_t bits) {
  __analogSetWidth(bits);
}

// extern int hallRead() {
  // return __hallRead();
// }
#endif
