// Copyright 2025 - 2026 the original author or authors.
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

#include "time_utils.h"

// function buffering delay() 
// arduino uno function doesn't work well with interrupts
inline void _delay(unsigned long ms) {
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328PB__)  || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
  // if arduino uno and other atmega328p chips
  // use while instad of delay, 
  // due to wrong measurement based on changed timer0
  unsigned long t = _micros() + ms * 1000;
  while (_micros() < t) {
  };
#else
  // regular micros
  delay(ms);
#endif
}


// function buffering _micros() 
// arduino function doesn't work well with interrupts
inline unsigned long _micros() {
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328PB__)  || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
  // if arduino uno and other atmega328p chips
  //return the value based on the prescaler
  if ((TCCR0B & 0b00000111) == 0x01) return (micros() / 32);
  else return (micros());
#else
  // regular micros
  return micros();
#endif
}