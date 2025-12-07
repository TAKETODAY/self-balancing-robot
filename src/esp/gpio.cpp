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

#include "esp/gpio.hpp"
#include "soc/soc_caps.h"

typedef struct {
  gpio_config_t conf; /*!< gpio pin configuration */
  gpio_num_t number; /*!< gpio pin number */
} IoRecord;

static IoRecord ioRecord[GPIO_NUM_MAX];

void pinMode(const int pin, const uint8_t mode) {
  if (pin == GPIO_NUM_MAX || pin == GPIO_NUM_NC) {
    return;
  }

  ioRecord[pin].number = static_cast<gpio_num_t>(pin);
  ioRecord[pin].conf.pin_bit_mask = (1ULL << pin);
  ioRecord[pin].conf.intr_type = GPIO_INTR_DISABLE; // disable interrupt

  switch (mode) {
    case INPUT:
      ioRecord[pin].conf.mode = GPIO_MODE_INPUT;
      ioRecord[pin].conf.pull_up_en = GPIO_PULLUP_DISABLE;
      ioRecord[pin].conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;
    case OUTPUT:
      ioRecord[pin].conf.mode = GPIO_MODE_OUTPUT;
      ioRecord[pin].conf.pull_up_en = GPIO_PULLUP_DISABLE;
      ioRecord[pin].conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;
    case INPUT_PULLUP:
      ioRecord[pin].conf.mode = GPIO_MODE_INPUT;
      ioRecord[pin].conf.pull_up_en = GPIO_PULLUP_ENABLE;
      ioRecord[pin].conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;
    case INPUT_PULLDOWN:
      ioRecord[pin].conf.mode = GPIO_MODE_INPUT;
      ioRecord[pin].conf.pull_up_en = GPIO_PULLUP_DISABLE;
      ioRecord[pin].conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
      break;
    default:
      break;
  }

  gpio_config(&ioRecord[pin].conf);
}

void digitalWrite(const int pin, const uint8_t val) {
  if (pin == GPIO_NUM_MAX || pin == GPIO_NUM_NC) {
    return;
  }
  gpio_set_level(ioRecord[pin].number, val);
}

int digitalRead(int pin) {
  return gpio_get_level(ioRecord[pin].number);
}

uint8_t digitalPinToInterrupt(const int pin) {
  return pin;
}

void attachInterrupt(const int pin, const gpio_isr_t handler, const int mode) {
  if (pin == GPIO_NUM_MAX || pin == GPIO_NUM_NC) {
    return;
  }

  switch (mode) {
    case RISING:
      ioRecord[pin].conf.intr_type = GPIO_INTR_POSEDGE;
      break;
    case FALLING:
      ioRecord[pin].conf.intr_type = GPIO_INTR_NEGEDGE;
      break;
    case CHANGE:
      ioRecord[pin].conf.intr_type = GPIO_INTR_ANYEDGE;
      break;
    default:
      break;
  }

  gpio_config(&ioRecord[pin].conf);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(ioRecord[pin].number, handler, reinterpret_cast<void*>(ioRecord[pin].number));
}

//

// It fixes lack of pin definition for S3 and for any future SoC
// this function works for ESP32, ESP32-S2 and ESP32-S3 - including the C3, it will return -1 for any pin
#if SOC_TOUCH_SENSOR_NUM >  0
#include "soc/touch_sensor_periph.h"

int8_t digitalPinToTouchChannel(uint8_t pin) {
  int8_t ret = -1;
  if (pin < SOC_GPIO_PIN_COUNT) {
    for (uint8_t i = 0; i < SOC_TOUCH_SENSOR_NUM; i++) {
      if (touch_sensor_channel_io_map[i] == pin) {
        ret = i;
        break;
      }
    }
  }
  return ret;
}
#else
// No Touch Sensor available
int8_t digitalPinToTouchChannel(uint8_t pin) {
  return -1;
}
#endif

#ifdef SOC_ADC_SUPPORTED
#include "soc/adc_periph.h"

int8_t digitalPinToAnalogChannel(uint8_t pin) {
  uint8_t channel = 0;
  if (pin < SOC_GPIO_PIN_COUNT) {
    for (uint8_t i = 0; i < SOC_ADC_PERIPH_NUM; i++) {
      for (uint8_t j = 0; j < SOC_ADC_MAX_CHANNEL_NUM; j++) {
        if (adc_channel_io_map[i][j] == pin) {
          return channel;
        }
        channel++;
      }
    }
  }
  return -1;
}

int8_t analogChannelToDigitalPin(uint8_t channel) {
  if (channel >= (SOC_ADC_PERIPH_NUM * SOC_ADC_MAX_CHANNEL_NUM)) {
    return -1;
  }
  uint8_t adc_unit = (channel / SOC_ADC_MAX_CHANNEL_NUM);
  uint8_t adc_chan = (channel % SOC_ADC_MAX_CHANNEL_NUM);
  return adc_channel_io_map[adc_unit][adc_chan];
}
#else
// No Analog channels availible
int8_t analogChannelToDigitalPin(uint8_t channel) {
  return -1;
}
#endif
