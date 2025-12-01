#pragma once

#define LED_BATTERY_PIN 13
#define BAT_PIN 35

void battery_init();

float battery_voltage_read();
