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

#include "ble/log_service.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <string.h>

// 服务结构
struct ble_log_service_t {
  ble_log_forwarder_t* forwarder; // 日志转发器
  uint16_t svc_handle;            // 服务句柄
  uint16_t chr_handle;            // 特征句柄
  bool is_connected;              // 连接状态
};

// 全局实例
static ble_log_service_t* g_log_service = NULL;

// BLE服务定义
static const ble_uuid128_t gatt_svc_uuid = {
  .u = { .type = BLE_UUID_TYPE_128 },
  .value = {
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0xA0, 0xF0, 0x00, 0x00
  }
};

static const ble_uuid128_t gatt_chr_uuid = {
  .u = { .type = BLE_UUID_TYPE_128 },
  .value = {
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0xA1, 0xF0, 0x00, 0x00
  }
};

// 日志数据回调
static void on_log_ready(const char* log_line) {
  if (!g_log_service || !g_log_service->is_connected) {
    return;
  }

  // 通过BLE发送日志
  struct os_mbuf* om = ble_hs_mbuf_from_flat(log_line, strlen(log_line));
  if (!om) {
    return;
  }

  ble_gatts_notify_custom(g_log_service->svc_handle,
    g_log_service->chr_handle,
    om);
}

// 错误回调
static void on_log_error(int error_code, const char* error_msg) {
  // 这里可以处理错误，比如记录到其他位置
  (void) error_code;
  (void) error_msg;
}

// BLE事件处理
static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
  struct ble_gatt_access_ctxt* ctxt, void* arg) {
  (void) conn_handle;
  (void) attr_handle;
  (void) arg;

  switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
      // 可以返回一些状态信息
      break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
      // 可以接收配置更新
      break;

    default:
      break;
  }

  return 0;
}

// GATT服务
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
  {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svc_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[]){
      {
        .uuid = &gatt_chr_uuid.u,
        .access_cb = gatt_svr_chr_access,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
        .val_handle = &g_log_service->chr_handle,
      },
      { 0 }
    }
  },
  { 0 }
};

// BLE事件回调
static void ble_app_on_sync(void) {
  // 服务注册
  int rc = ble_gatts_count_cfg(gatt_svr_svcs);
  rc += ble_gatts_add_svcs(gatt_svr_svcs);
  assert(rc == 0);
}

static void ble_app_on_reset(int reason) {
  (void) reason;
}

static void ble_app_on_connect(struct ble_gap_event* event, void* arg) {
  (void) arg;

  if (g_log_service) {
    g_log_service->is_connected = true;
  }
}

static void ble_app_on_disconnect(struct ble_gap_event* event, void* arg) {
  (void) arg;

  if (g_log_service) {
    g_log_service->is_connected = false;
  }
}

static int ble_app_gap_event(struct ble_gap_event* event, void* arg) {
  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      ble_app_on_connect(event, arg);
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      ble_app_on_disconnect(event, arg);
      break;

    default:
      break;
  }

  return 0;
}

// 初始化BLE日志服务
ble_log_service_t* ble_log_service_init(void) {
  if (g_log_service) {
    return g_log_service; // 已经初始化
  }

  g_log_service = (ble_log_service_t*) malloc(sizeof(ble_log_service_t));
  if (!g_log_service) {
    return NULL;
  }

  memset(g_log_service, 0, sizeof(ble_log_service_t));

  // 初始化日志转发器
  ble_log_config_t config = {
    .include_timestamp = true,
    .include_tag = true,
    .include_level = true,
    .include_color = false,
    .include_file_line = false,
    .max_line_length = 200,
    .filter_level = BLE_LOG_LEVEL_INFO
  };

  g_log_service->forwarder = ble_log_forwarder_init(&config);
  if (!g_log_service->forwarder) {
    free(g_log_service);
    return NULL;
  }

  // 设置回调
  ble_log_forwarder_set_callbacks(g_log_service->forwarder,
    on_log_ready,
    on_log_error);

  return g_log_service;
}

// 启动BLE日志服务
bool ble_log_service_start(ble_log_service_t* service) {
  if (!service) return false;

  // 启动BLE主机栈
  nimble_port_init();

  // 设置设备名称
  ble_svc_gap_device_name_set("ESP32-Logger");

  // 注册GAP事件回调
  ble_gap_event_listener_register(&ble_app_gap_event, NULL, NULL);

  // 设置同步回调
  ble_hs_cfg.sync_cb = ble_app_on_sync;
  ble_hs_cfg.reset_cb = ble_app_on_reset;

  // 启动BLE主机任务
  nimble_port_freertos_init([](void*param) {
    nimble_port_run();
    vTaskDelete(NULL);
  }
  )
  ;

  // 启动日志转发器
  if (!ble_log_forwarder_start(service->forwarder)) {
    return false;
  }

  return true;
}

// 停止BLE日志服务
void ble_log_service_stop(ble_log_service_t* service) {
  if (!service) return;

  // 停止日志转发器
  ble_log_forwarder_stop(service->forwarder);

  // 停止BLE
  nimble_port_stop();
}

// 销毁BLE日志服务
void ble_log_service_deinit(ble_log_service_t* service) {
  if (!service) return;

  ble_log_service_stop(service);

  if (service->forwarder) {
    ble_log_forwarder_deinit(service->forwarder);
  }

  free(service);
  g_log_service = NULL;
}

// 设置日志配置
bool ble_log_service_set_config(ble_log_service_t* service,
  const ble_log_config_t* config) {
  if (!service || !service->forwarder || !config) {
    return false;
  }

  return ble_log_forwarder_update_config(service->forwarder, config);
}

// 获取当前配置
const ble_log_config_t* ble_log_service_get_config(ble_log_service_t* service) {
  if (!service || !service->forwarder) {
    return NULL;
  }

  return ble_log_forwarder_get_config(service->forwarder);
}

// 获取日志转发器
ble_log_forwarder_t* ble_log_service_get_forwarder(ble_log_service_t* service) {
  if (!service) return NULL;
  return service->forwarder;
}

// 检查是否有客户端连接
bool ble_log_service_is_connected(ble_log_service_t* service) {
  if (!service) return false;
  return service->is_connected;
}
