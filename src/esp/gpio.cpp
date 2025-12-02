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

static IoRecord ioRecord[GPIO_NUM_MAX];

void pinMode(const gpio_num_t pin, const PinMode mode) {
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

void digitalWrite(const gpio_num_t pin, const PinLevel val) {
  if (pin == GPIO_NUM_MAX || pin == GPIO_NUM_NC) {
    return;
  }
  gpio_set_level(ioRecord[pin].number, val);
}

int digitalRead(gpio_num_t pin) {
  return gpio_get_level(ioRecord[pin].number);
}

uint8_t digitalPinToInterrupt(const gpio_num_t pin) {
  return pin;
}

void attachInterrupt(const gpio_num_t pin, const gpio_isr_t handler, const int mode) {
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
