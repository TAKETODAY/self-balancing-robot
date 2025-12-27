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

#include "ble/led.h"
#include "ble/common.h"

/* Private variables */
static uint8_t led_state;

/* Public functions */
uint8_t get_led_state(void) { return led_state; }


void led_on(void) {

  // gpio_set_level(CONFIG_BLINK_GPIO, true);
}

void led_off(void) {
  // gpio_set_level(CONFIG_BLINK_GPIO, false);
}

void led_init(void) {
  ESP_LOGI(TAG, "example configured to blink gpio led!");
  // gpio_reset_pin(CONFIG_BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  // gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
}
