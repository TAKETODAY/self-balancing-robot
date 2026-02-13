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


#pragma once

#include "defs.h"
#include "../buffer.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {



#endif

typedef enum : uint8_t {
  status_battery = 1,
  status_robot_height = 2,

} status_type_t;

typedef struct {
  status_type_t type;

  union {
    percentage_t battery;
    percentage_t robot_height;
  };

} status_report_t;

bool status_report_serialize(status_report_t* msg, buffer_t* buf);
bool status_report_deserialize(status_report_t* msg, buffer_t* buf);

status_report_t status_report_create(status_type_t type);

#ifdef __cplusplus
}
#endif
