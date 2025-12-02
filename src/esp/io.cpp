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

#include <string.h>
#include <sstream>
#include <iomanip>
#include "esp/platform.hpp"
#include "esp/io.hpp"

size_t Writer::write(const uint8_t* buffer, size_t size) {
  if (buffer == nullptr) {
    return 0;
  }

  size_t n = 0;
  while (size--) {
    n += write(*buffer++);
  }
  return n;
}

size_t Writer::write(const char* str) {
  if (str == nullptr) {
    return 0;
  }
  return write((const uint8_t*) str, strlen(str));
}

size_t Writer::write(const char* buffer, size_t size) {
  return write((const uint8_t*) buffer, size);
}

size_t Writer::print(double n, int digits) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(digits) << n;
  std::string str = oss.str();

  return write(str.c_str());
}

size_t Writer::println(void) {
  return print("\r\n");
}

size_t Writer::println(double num, int digits) {
  size_t n = print(num, digits);
  n += println();
  return n;
}

Stream::Stream() {
  _startMillis = 0;
  _timeout = 0;
}

int Stream::timedRead() {
  int c;
  _startMillis = millis();
  do {
    c = read();
    if (c >= 0) {
      return c;
    }
  } while (millis() - _startMillis < _timeout);
  return -1;
}

size_t Stream::readBytes(char* buffer, size_t length) {
  size_t count = 0;
  while (count < length) {
    int c = timedRead();
    if (c < 0) {
      break;
    }
    *buffer++ = (char) c;
    count++;
  }
  return count;
}

size_t Stream::readBytes(uint8_t* buffer, size_t length) {
  return readBytes((char*) buffer, length);
}

void Stream::setTimeout(unsigned long timeout) {
  _timeout = timeout;
}

unsigned long Stream::getTimeout() {
  return _timeout;
}
