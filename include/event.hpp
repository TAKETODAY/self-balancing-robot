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

#include <map>

#include "logging.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
  EVENT_JUMP_REQUESTED,
  EVENT_CROUCH_REQUESTED,
  EVENT_STAND_UP_REQUESTED,
  EVENT_MOVE_FORWARD,
  EVENT_MOVE_BACKWARD,
  EVENT_TURN_LEFT,
  EVENT_TURN_RIGHT,
  EVENT_EMERGENCY_STOP,
} RobotEventType;

typedef struct {
  RobotEventType type;

  union {
    float float_param;
    int int_param;
    void* ptr_param;
  } data;
} RobotEvent;

typedef void (*RobotEventHandler)(const RobotEvent* event);

class EventDispatcher {

public:
  explicit EventDispatcher(const uint32_t queue_length = 20) {
    event_queue_ = xQueueCreate(queue_length, sizeof(RobotEvent));
    handler_map_.clear();
    // 创建专用任务来处理事件（中低优先级）
    xTaskCreate(dispatcher_task, "evt_disp", 4096, this, 5, &dispatcher_task_handle_);
  }

  bool send(const RobotEvent& event, TickType_t timeout = portMAX_DELAY) {
    return xQueueSend(event_queue_, &event, timeout) == pdTRUE;
  }

  bool send_event_from_isr(const RobotEvent& event) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    bool success = xQueueSendFromISR(event_queue_, &event, &xHigherPriorityTaskWoken) == pdTRUE;
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
    return success;
  }

  void register_handler(const RobotEventType event_type, const RobotEventHandler handler) {
    handler_map_[event_type] = handler;
  }

private:
  QueueHandle_t event_queue_;
  TaskHandle_t dispatcher_task_handle_;

  std::map<RobotEventType, RobotEventHandler> handler_map_;


  [[noreturn]]
  static void dispatcher_task(void* arg) {
    auto* self = static_cast<EventDispatcher*>(arg);
    RobotEvent event;
    while (true) {
      // 阻塞等待事件到来
      if (xQueueReceive(self->event_queue_, &event, portMAX_DELAY)) {
        auto it = self->handler_map_.find(event.type);
        if (it != self->handler_map_.end()) {
          // 找到注册的处理器，调用它
          it->second(&event);
        }
        else {
          static auto TAG = "Event-dispatcher";
          log_error("EVENT", "No handler for event type: %d", event.type);
        }
      }
    }
  }
};

extern EventDispatcher dispatcher;
