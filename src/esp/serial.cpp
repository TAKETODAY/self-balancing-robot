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

#include "esp/serial.hpp"

typedef struct {
  int8_t tx;
  int8_t rx;
} SerialPin;

static constexpr SerialPin serialPins[UART_NUM_MAX] = {
  { UART_NUM_0_TXD_DIRECT_GPIO_NUM, UART_NUM_0_RXD_DIRECT_GPIO_NUM },
  { UART_NUM_1_TXD_DIRECT_GPIO_NUM, UART_NUM_1_RXD_DIRECT_GPIO_NUM },
  { UART_NUM_2_TXD_DIRECT_GPIO_NUM, UART_NUM_2_RXD_DIRECT_GPIO_NUM },
};

SerialPort serial(UART_NUM_0);

SerialPort::SerialPort(const uart_port_t _uart_num) :
  _txPin(serialPins[_uart_num].tx),
  _rxPin(serialPins[_uart_num].rx),
  _uart_num(_uart_num),
  _rxBufferSize(256),
  _txBufferSize(0) {

}

SerialPort::~SerialPort() {
  if (uart_is_driver_installed(_uart_num)) {
    uart_driver_delete(_uart_num);
  }
}

size_t SerialPort::getRxBuffer() const {
  return _rxBufferSize;
}

uart_port_t SerialPort::getPortNumber() const {
  return _uart_num;
}

void SerialPort::setPortNumber(const uart_port_t port) {
  _uart_num = port;
}

void SerialPort::begin(const int baud, const uart_word_length_t wordLength, const uart_parity_t parity, const uart_stop_bits_t stopBits) {
  if (!uart_is_driver_installed(_uart_num)) {
    uart_config_t _uart_config{};
    _uart_config.baud_rate = baud;
    _uart_config.data_bits = wordLength;
    _uart_config.parity = parity;
    _uart_config.stop_bits = stopBits;
    _uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    _uart_config.source_clk = UART_SCLK_DEFAULT;

    uart_driver_install(_uart_num, _rxBufferSize * 2, 0, 0, NULL, 0);
    uart_param_config(_uart_num, &_uart_config);
    uart_set_pin(_uart_num, _txPin, _rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  }
}

size_t SerialPort::available() {
  size_t available;
  uart_get_buffered_data_len(_uart_num, &available);
  return available;
}

int SerialPort::peek() {
  return 1;
}

size_t SerialPort::write(uint8_t c) {
  uart_write_bytes(_uart_num, &c, 1);
  return 1;
}

size_t SerialPort::write(const uint8_t* buffer, size_t size) {
  uart_write_bytes(_uart_num, buffer, size);
  return size;
}

int SerialPort::read() {
  uint8_t c = 0;
  if (uart_read_bytes(_uart_num, &c, 1, 10 / portTICK_PERIOD_MS) == 1) {
    return c;
  }
  return -1;
}

size_t SerialPort::read(uint8_t* buffer, size_t size) const {
  return uart_read_bytes(_uart_num, buffer, size, 10 / portTICK_PERIOD_MS);
}

size_t SerialPort::readBytes(uint8_t* buffer, size_t length) {
  return uart_read_bytes(_uart_num, buffer, length, 10 / portTICK_PERIOD_MS);
}
