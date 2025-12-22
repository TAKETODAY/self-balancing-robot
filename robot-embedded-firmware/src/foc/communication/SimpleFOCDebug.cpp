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

#include "SimpleFOCDebug.h"

#ifndef SIMPLEFOC_DISABLE_DEBUG


Writer* SimpleFOCDebug::_debugPrint = nullptr;


void SimpleFOCDebug::enable(Writer* debugPrint) {
  _debugPrint = debugPrint;
}


void SimpleFOCDebug::println(int val) {
  if (_debugPrint != nullptr) {
    _debugPrint->println(val);
  }
}

void SimpleFOCDebug::println(float val) {
  if (_debugPrint != nullptr) {
    _debugPrint->println(val);
  }
}


void SimpleFOCDebug::println(const char* str) {
  if (_debugPrint != nullptr) {
    _debugPrint->println(str);
  }
}

void SimpleFOCDebug::println(const __FlashStringHelper* str) {
  if (_debugPrint != nullptr) {
    _debugPrint->println(str);
  }
}


void SimpleFOCDebug::println(const char* str, float val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
    _debugPrint->println(val);
  }
}

void SimpleFOCDebug::println(const __FlashStringHelper* str, float val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
    _debugPrint->println(val);
  }
}

void SimpleFOCDebug::println(const char* str, int val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
    _debugPrint->println(val);
  }
}

void SimpleFOCDebug::println(const char* str, char val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
    _debugPrint->println(val);
  }
}

void SimpleFOCDebug::println(const __FlashStringHelper* str, int val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
    _debugPrint->println(val);
  }
}


void SimpleFOCDebug::print(const char* str) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
  }
}


void SimpleFOCDebug::print(const __FlashStringHelper* str) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str);
  }
}

void SimpleFOCDebug::print(const std::string str) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(str.c_str());
  }
}


void SimpleFOCDebug::println(const std::string str) {
  if (_debugPrint != nullptr) {
    _debugPrint->println(str.c_str());
  }
}


void SimpleFOCDebug::print(int val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(val);
  }
}


void SimpleFOCDebug::print(float val) {
  if (_debugPrint != nullptr) {
    _debugPrint->print(val);
  }
}


void SimpleFOCDebug::println() {
  if (_debugPrint != nullptr) {
    _debugPrint->println();
  }
}

#endif
