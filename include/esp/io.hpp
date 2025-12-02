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

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <sstream>
#include <string.h>

class Writer {
public:
  /**
   * @brief Construct a new Writer object
   */
  Writer() {
  }

  /**
   * @brief Destroy the Writer object
   */
  virtual ~Writer() {
  }

  /**
   * @brief Write character.
   *
   * @return character size
   */
  virtual size_t write(uint8_t) = 0;

  /**
   * @brief Write buffer.
   *
   * @param str buffer
   * @return buffer size
   */
  size_t write(const char* str);

  /**
   * @brief Write buffer.
   *
   * @param buffer buffer
   * @param size buffer size
   * @return buffer size
   */
  virtual size_t write(const uint8_t* buffer, size_t size);

  /**
   * @brief Write buffer.
   *
   * @param buffer buffer
   * @param size buffer size
   * @return buffer size
   */
  size_t write(const char* buffer, size_t size);

  /**
   * @brief Write different types of data.
   *
   * @tparam T
   * @param value data
   * @return length of the converted string
   */
  template<typename T>
  size_t print(T value) {
    std::stringstream ss;
    ss << value;
    return write(ss.str().c_str());
  }

  /**
   * @brief Write data of type double.
   *
   * @return length of the converted string
   */
  size_t print(double, int = 2);

  /**
   * @brief Write different types of data and newline.
   *
   * @tparam T
   * @param value data
   * @return length of the converted string
   */
  template<typename T>
  size_t println(T value) {
    std::stringstream ss;
    ss << value << "\r\n";
    return write(ss.str().c_str());
  }

  /**
   * @brief Write data of type double.
   *
   * @return length of the converted string
   */
  size_t println(double, int = 2);

  /**
   * @brief Print newline.
   *
   * @return length of "\r\n"
   */
  size_t println();
};

class Stream : public Writer {
protected:
  unsigned long _timeout;
  unsigned long _startMillis;
  int timedRead();

public:
  /**
   * @brief Whether the RX buffer has data.
   *
   * @return
   *     - 0 have data
   *     - 1 no data
   */
  virtual size_t available() = 0;

  /**
   * @brief Read serial data.
   *
   * @return data size
   */
  virtual int read() = 0;

  /**
   * @brief Construct a new Stream object.
   *
   */
  Stream();

  /**
   * @brief Destroy the Stream object.
   *
   */
  ~Stream() override {
  }

  /**
   * @brief Set timeout.
   *
   * @param timeout timeout
   */
  void setTimeout(unsigned long timeout);

  /**
   * @brief Get timeout.
   *
   * @return timeout
   */
  unsigned long getTimeout();

  /**
   * @brief Read bytes.
   *
   * @param buffer pointer to the buffer.
   * @param length data length
   * @return data length
   */
  virtual size_t readBytes(char* buffer, size_t length);

  /**
   * @brief Read bytes.
   *
   * @param buffer pointer to the buffer.
   * @param length data length
   * @return data length
   */
  virtual size_t readBytes(uint8_t* buffer, size_t length);
};
