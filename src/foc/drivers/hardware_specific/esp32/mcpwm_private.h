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

#ifndef MCPWM_PRIVATE_H
#define MCPWM_PRIVATE_H


#include "freertos/FreeRTOS.h"
#include "esp_intr_alloc.h"
#include "esp_heap_caps.h"
#include "esp_pm.h"
#include "soc/soc_caps.h"
#include "hal/mcpwm_hal.h"
#include "hal/mcpwm_types.h"
#include "driver/mcpwm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mcpwm_group_t mcpwm_group_t;
typedef struct mcpwm_timer_t mcpwm_timer_t;
typedef struct mcpwm_cap_timer_t mcpwm_cap_timer_t;
typedef struct mcpwm_oper_t mcpwm_oper_t;
typedef struct mcpwm_gpio_fault_t mcpwm_gpio_fault_t;
typedef struct mcpwm_gpio_sync_src_t mcpwm_gpio_sync_src_t;
typedef struct mcpwm_timer_sync_src_t mcpwm_timer_sync_src_t;

struct mcpwm_group_t {
  int group_id; // group ID, index from 0
  int intr_priority; // MCPWM interrupt priority
  mcpwm_hal_context_t hal; // HAL instance is at group level
  portMUX_TYPE spinlock; // group level spinlock
  uint32_t prescale; // group prescale
  uint32_t resolution_hz; // MCPWM group clock resolution: clock_src_hz / clock_prescale = resolution_hz
  esp_pm_lock_handle_t pm_lock; // power management lock
  soc_module_clk_t clk_src; // peripheral source clock
  mcpwm_cap_timer_t* cap_timer; // mcpwm capture timers
  mcpwm_timer_t* timers[SOC_MCPWM_TIMERS_PER_GROUP]; // mcpwm timer array
  mcpwm_oper_t* operators[SOC_MCPWM_OPERATORS_PER_GROUP]; // mcpwm operator array
  mcpwm_gpio_fault_t* gpio_faults[SOC_MCPWM_GPIO_FAULTS_PER_GROUP]; // mcpwm fault detectors array
  mcpwm_gpio_sync_src_t* gpio_sync_srcs[SOC_MCPWM_GPIO_SYNCHROS_PER_GROUP]; // mcpwm gpio sync array
};

typedef enum {
  MCPWM_TIMER_FSM_INIT,
  MCPWM_TIMER_FSM_ENABLE,
} mcpwm_timer_fsm_t;

struct mcpwm_timer_t {
  int timer_id; // timer ID, index from 0
  mcpwm_group_t* group; // which group the timer belongs to
  mcpwm_timer_fsm_t fsm; // driver FSM
  portMUX_TYPE spinlock; // spin lock
  intr_handle_t intr; // interrupt handle
  uint32_t resolution_hz; // resolution of the timer
  uint32_t peak_ticks; // peak ticks that the timer could reach to
  mcpwm_timer_sync_src_t* sync_src; // timer sync_src
  mcpwm_timer_count_mode_t count_mode; // count mode
  mcpwm_timer_event_cb_t on_full; // callback function when MCPWM timer counts to peak value
  mcpwm_timer_event_cb_t on_empty; // callback function when MCPWM timer counts to zero
  mcpwm_timer_event_cb_t on_stop; // callback function when MCPWM timer stops
  void* user_data; // user data which would be passed to the timer callbacks
};

#ifdef __cplusplus
}
#endif


#endif /* MCPWM_PRIVATE_H */
