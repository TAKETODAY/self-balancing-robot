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

#include "esp_log.h"

// static const char* TAG = "default";

// #define log_info(TAG, format, ...) ESP_LOGI(TAG, format, __VA_ARGS__)
// #define log_debug(TAG, format, ...) ESP_LOGD(TAG, format, __VA_ARGS__)
// #define log_warn(TAG, format, ...) ESP_LOGW(TAG, format, __VA_ARGS__)
// #define log_trace(TAG, format, ...) ESP_LOGV(TAG, format, __VA_ARGS__)
// #define log_error(TAG, format, ...) ESP_LOGE(TAG, format, __VA_ARGS__)
//

#define log_info(format, ...) ESP_LOGI(TAG, format, __VA_ARGS__)
#define log_debug(format, ...) ESP_LOGD(TAG, format, __VA_ARGS__)
#define log_warn(format, ...) ESP_LOGW(TAG, format, __VA_ARGS__)
#define log_trace(format, ...) ESP_LOGV(TAG, format, __VA_ARGS__)
#define log_error(format, ...) ESP_LOGE(TAG, format, __VA_ARGS__)
