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

static bool serialize_body(status_report_t* msg, buffer_t* buf) {
  switch (msg->type) {
    case status_battery: return buffer_write_u8(buf, msg->battery.percentage);
    case status_robot_height: return buffer_write_u8(buf, msg->robot_height.percentage);

  }
  return false;
}

bool status_report_serialize(status_report_t* msg, buffer_t* buf) {
  return buffer_write_u8(buf, msg->type)
         && serialize_body(msg, buf);
}

static bool deserialize_body(status_report_t* msg, buffer_t* buf) {
  switch (msg->type) {
    case status_battery: return buffer_read_u8(buf, &msg->battery.percentage);
    case status_robot_height: return buffer_read_u8(buf, &msg->robot_height.percentage);
  }
  return false;
}

bool status_report_deserialize(status_report_t* msg, buffer_t* buf) {
  return buffer_read_u8(buf, (uint8_t*) &msg->type)
         && deserialize_body(msg, buf);
}

inline status_report_t status_report_create(const status_type_t type) {
  return (status_report_t){
    .type = type
  };
}
