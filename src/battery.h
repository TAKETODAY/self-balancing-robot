#pragma once

#include <esp_adc_cal.h>

#define LED_BATTERY_PIN 13
#define BAT_PIN 35

static const adc1_channel_t channel = ADC1_CHANNEL_7;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12; // ADC采样宽度
static const adc_atten_t atten = ADC_ATTEN_DB_12; // ADC衰减系数
static const adc_unit_t unit = ADC_UNIT_1; // ADC单元

void battery_init();

float battery_voltage_read();
