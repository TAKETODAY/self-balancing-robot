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


#include "protocol/message/status_report.h"

bool status_report_serialize(status_report_t* msg, buffer_t* buf) {
  buffer_write_u8(buf, msg->type);
  switch (msg->type) {
    case status_robot_height:
      return buffer_write_u8(buf, msg->battery.percentage);
    default: return false;
  }
}

bool status_report_deserialize(status_report_t* msg, buffer_t* buf) {

  return false;
}

inline status_report_t status_report_create(status_type_t type) {
  return (status_report_t){
    .type = type
  };
}
