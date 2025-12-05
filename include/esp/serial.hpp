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

#include "io.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/soc_caps.h"
#include "soc/uart_channel.h"

#include <cstring>

/**
 * ESP32-S-Devkitc default serial port: tx:1 rx:3
 * ESP32-S3-Devkitc default serial port: tx:43 rx:44
 */

#define TX0 UART_NUM_0_TXD_DIRECT_GPIO_NUM
#define RX0 UART_NUM_0_RXD_DIRECT_GPIO_NUM

class SerialPort final : public Stream {
public:
  /**
   * @brief Construct a new Serial object
   */
  SerialPort(uart_port_t _uart_num);

  /**
   * @brief Destroy the SerialPort object
   */
  ~SerialPort() override;

  /**
   * @brief SerialPort begin function, support separate setting of serial port and IO.
   *
   * @param baud uart baud rate.
   * @param wordLength  uart word length constants
   * @param parity uart parity constants
   * @param stopBits uart stop bits number
   */
  void begin(int baud, uart_word_length_t wordLength = UART_DATA_8_BITS,
    uart_parity_t parity = UART_PARITY_DISABLE, uart_stop_bits_t stopBits = UART_STOP_BITS_1);

  /**
   * @brief Whether the RX buffer has data.
   *
   * @return
   *     - 0 have data
   *     - 1 no data
   */
  size_t available() override;

  /**
   * @brief Empty Function.
   *
   * @return always return 1
   * @note this function does no processing.
   */
  int peek();

  /**
   * @brief Get rx buffer size.
   *
   * @return buffer size
   */
  [[nodiscard]]
  size_t getRxBuffer() const;

  /**
   * @brief Get uart port.
   *
   * @return uart port
   */
  uart_port_t getPortNumber() const;

  void setPortNumber(uart_port_t port);

  /**
   * @brief Send data to the UART port from a given character.
   *
   * @return
   *     - (-1) Parameter error
   *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
   */
  size_t write(uint8_t) override;

  /**
   * @brief Send data to the UART port from a given buffer and length.
   *
   * @param buffer data buffer address
   * @param size data length to send
   * @return
   *     - (-1) Parameter error
   *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
   */
  size_t write(const uint8_t* buffer, size_t size) override;

  /**
   * @brief Send data to the UART port from a given buffer and length.
   *
   * @param buffer data buffer address
   * @param size data length to send
   * @return
   *     - (-1) Parameter error
   *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
   */
  inline size_t write(const char* buffer, size_t size) {
    return write((uint8_t*) buffer, size);
  }

  /**
   * @brief Send data to the UART port from a given buffer.
   *
   * @param buffer data buffer address
   * @return
   *     - (-1) Parameter error
   *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
   */
  inline size_t write(const char* buffer) {
    return write((uint8_t*) buffer, strlen(buffer));
  }

  /**
   * @brief Send data to the UART port from a given data.
   *
   * @param n data
   * @return data length
   */
  inline size_t write(unsigned long n) {
    return write(static_cast<uint8_t>(n));
  }

  /**
   * @brief Send data to the UART port from a given data.
   *
   * @param n data
   * @return data length
   */
  inline size_t write(long n) {
    return write(static_cast<uint8_t>(n));
  }

  /**
   * @brief Send data to the UART port from a given data.
   *
   * @param n data
   * @return data length
   */
  inline size_t write(unsigned int n) {
    return write(static_cast<uint8_t>(n));
  }

  /**
   * @brief Send data to the UART port from a given data.
   *
   * @param n data
   * @return data length
   */
  inline size_t write(int n) {
    return write(static_cast<uint8_t>(n));
  }

  /**
   * @brief UART read byte from UART buffer.
   *
   * @return
   *     - (-1) Error
   *     - OTHERS (>=0) The number of bytes read from UART buffer
   */
  int read() override;

  /**
   * @brief UART read bytes from UART buffer.
   *
   * @param buffer pointer to the buffer
   * @param size data length
   * @return
   *     - (-1) Error
   *     - OTHERS (>=0) The number of bytes read from UART buffer
   */
  size_t read(uint8_t* buffer, size_t size) const;

  /**
   * @brief UART read bytes from UART buffer.
   *
   * @param buffer pointer to the buffer
   * @param length data length
   * @return
   *     - (-1) Error
   *     - OTHERS (>=0) The number of bytes read from UART buffer
   */
  size_t readBytes(uint8_t* buffer, size_t length) override;

  /**
   * @brief UART read bytes from UART buffer.
   *
   * @param buffer pointer to the buffer
   * @param length data length
   * @return
   *     - (-1) Error
   *     - OTHERS (>=0) The number of bytes read from UART buffer
   */
  size_t readBytes(char* buffer, const size_t length) override {
    return readBytes(reinterpret_cast<uint8_t*>(buffer), length);
  }

private:
  int8_t _rxPin, _txPin;
  uart_port_t _uart_num;
  size_t _rxBufferSize;
  size_t _txBufferSize;

};
