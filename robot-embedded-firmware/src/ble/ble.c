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

#include "esp_log.h"
#include "nvs_flash.h"
/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble/spp_server.h"
#include "ble.h"

#define BLE_RX_QUEUE_LEN 10
#define BLE_RX_ITEM_SIZE 128 // 根据你的协议最大长度调整

static int ble_spp_server_gap_event(struct ble_gap_event* event, void* arg);
static uint8_t own_addr_type;
int gatt_svr_register(void);
QueueHandle_t spp_common_uart_queue = NULL;
static bool conn_handle_subs[CONFIG_BT_NIMBLE_MAX_CONNECTIONS + 1];
static uint16_t ble_spp_svc_gatt_read_val_handle;

void ble_store_config_init(void);

/**
 * Logs information about a connection to the console.
 */
static void ble_spp_server_print_conn_desc(struct ble_gap_conn_desc* desc) {
  MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
    desc->conn_handle, desc->our_ota_addr.type);
  print_addr(desc->our_ota_addr.val);
  MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
    desc->our_id_addr.type);
  print_addr(desc->our_id_addr.val);
  MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
    desc->peer_ota_addr.type);
  print_addr(desc->peer_ota_addr.val);
  MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
    desc->peer_id_addr.type);
  print_addr(desc->peer_id_addr.val);
  MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
    "encrypted=%d authenticated=%d bonded=%d\n",
    desc->conn_itvl, desc->conn_latency,
    desc->supervision_timeout,
    desc->sec_state.encrypted,
    desc->sec_state.authenticated,
    desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void ble_spp_server_advertise(void) {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields = { 0 };

  /**
   *  Set the advertisement data included in our advertisements:
   *     o Flags (indicates advertisement type and other general info).
   *     o Advertising tx power.
   *     o Device name.
   *     o 16-bit service UUIDs (alert notifications).
   */

  /* Advertise two flags:
   *     o Discoverability in forthcoming advertisement (general)
   *     o BLE-only (BR/EDR unsupported).
   */
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  /* Indicate that the TX power level field should be included; have the
   * stack fill this value automatically.  This is done by assigning the
   * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
   */
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  const char* name = ble_svc_gap_device_name();
  fields.name = (uint8_t*) name;
  fields.name_len = strlen(name);
  fields.name_is_complete = 1;

  fields.uuids16 = (ble_uuid16_t[]){
    BLE_UUID16_INIT(BLE_SVC_SPP_UUID16)
  };
  fields.num_uuids16 = 1;
  fields.uuids16_is_complete = 1;

  MODLOG_DFLT(INFO, "setting advertisement data; 0x%04X\n", BLE_SVC_SPP_UUID16);

  int rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
    return;
  }

  /* Begin advertising. */
  memset(&adv_params, 0, sizeof adv_params);
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_spp_server_gap_event, NULL);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
  }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * ble_spp_server uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param arg                   Application-specified argument; unused by
 *                                  ble_spp_server.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int ble_spp_server_gap_event(struct ble_gap_event* event, void* arg) {
  struct ble_gap_conn_desc desc;
  int rc;

  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      /* A new connection was established or a connection attempt failed. */
      MODLOG_DFLT(INFO, "connection %s; status=%d ",
        event->connect.status == 0 ? "established" : "failed",
        event->connect.status);
      if (event->connect.status == 0) {
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
        assert(rc == 0);
        ble_spp_server_print_conn_desc(&desc);
      }
      MODLOG_DFLT(INFO, "\n");
      if (event->connect.status != 0 || CONFIG_BT_NIMBLE_MAX_CONNECTIONS > 1) {
        /* Connection failed or if multiple connection allowed; resume advertising. */
        ble_spp_server_advertise();
      }
      return 0;

    case BLE_GAP_EVENT_DISCONNECT:
      MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
      ble_spp_server_print_conn_desc(&event->disconnect.conn);
      MODLOG_DFLT(INFO, "\n");

      conn_handle_subs[event->disconnect.conn.conn_handle] = false;

      /* Connection terminated; resume advertising. */
      ble_spp_server_advertise();
      return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
      /* The central has updated the connection parameters. */
      MODLOG_DFLT(INFO, "connection updated; status=%d ",
        event->conn_update.status);
      rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
      assert(rc == 0);
      ble_spp_server_print_conn_desc(&desc);
      MODLOG_DFLT(INFO, "\n");
      return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
      MODLOG_DFLT(INFO, "advertise complete; reason=%d",
        event->adv_complete.reason);
      ble_spp_server_advertise();
      return 0;

    case BLE_GAP_EVENT_MTU:
      MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
        event->mtu.conn_handle,
        event->mtu.channel_id,
        event->mtu.value);
      return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
      MODLOG_DFLT(INFO, "subscribe event; conn_handle=%d attr_handle=%d "
        "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
        event->subscribe.conn_handle,
        event->subscribe.attr_handle,
        event->subscribe.reason,
        event->subscribe.prev_notify,
        event->subscribe.cur_notify,
        event->subscribe.prev_indicate,
        event->subscribe.cur_indicate);
      conn_handle_subs[event->subscribe.conn_handle] = true;
      return 0;

    default:
      return 0;
  }
}

static void ble_spp_server_on_reset(int reason) {
  MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void ble_spp_server_on_sync(void) {
  int rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  /* Figure out address to use while advertising (no privacy for now) */
  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
    return;
  }

  /* Printing ADDR */
  uint8_t addr_val[6] = { 0 };
  rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

  MODLOG_DFLT(INFO, "Device Address: ");
  print_addr(addr_val);
  MODLOG_DFLT(INFO, "\n");
  /* Begin advertising. */
  ble_spp_server_advertise();
}

void ble_spp_server_host_task(void* param) {
  MODLOG_DFLT(INFO, "BLE Host Task Started");
  /* This function will return only when nimble_port_stop() is executed */
  nimble_port_run();

  nimble_port_freertos_deinit();
}

typedef struct {

} robot_control_frame_t;

/* Callback function for custom service */
static int ble_svc_gatt_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
      MODLOG_DFLT(INFO, "Callback for read");
      break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
      // 1. 获取数据指针和长度
      uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om); // 数据总长度
      uint8_t* om_data = ctxt->om->om_data; // 指向数据的指针

      uint8_t rx_buffer[BLE_RX_ITEM_SIZE];
      const size_t copy_len = om_len > BLE_RX_ITEM_SIZE ? BLE_RX_ITEM_SIZE : om_len;
      memcpy(rx_buffer, om_data, copy_len);

      if (xQueueSend(spp_common_uart_queue, rx_buffer, 0) != pdTRUE) {
        MODLOG_DFLT(WARN, "BLE RX 队列已满，丢弃数据包！\n");
      }
      else {
        MODLOG_DFLT(INFO, "数据包已放入队列等待处理。\n");
      }

      break;

    default:
      MODLOG_DFLT(INFO, "\nDefault Callback");
      break;
  }
  return 0;
}

/* Define new custom service */
static const struct ble_gatt_svc_def new_ble_svc_gatt_defs[] = {
  {
    /*** Service: SPP */
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(BLE_SVC_SPP_UUID16),
    .characteristics = (struct ble_gatt_chr_def[])
    {
      {
        /* Support SPP service */
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_SPP_CHR_UUID16),
        .access_cb = ble_svc_gatt_handler,
        .val_handle = &ble_spp_svc_gatt_read_val_handle,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
      },
      {
        0, /* No more characteristics */
      }
    },
  },
  {
    0, /* No more services. */
  },
};

static void gatt_svr_register_cb(struct ble_gatt_register_ctxt* ctxt, void* arg) {
  char buf[BLE_UUID_STR_LEN];

  switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
      MODLOG_DFLT(DEBUG, "registered service %s with handle=%d",
        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
      break;

    case BLE_GATT_REGISTER_OP_CHR:
      MODLOG_DFLT(DEBUG, "registering characteristic %s with def_handle=%d val_handle=%d",
        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf), ctxt->chr.def_handle, ctxt->chr.val_handle);
      break;

    case BLE_GATT_REGISTER_OP_DSC:
      MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d",
        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf), ctxt->dsc.handle);
      break;

    default:
      assert(0);
      break;
  }
}

int gatt_svr_init(void) {
  int rc = 0;
  ble_svc_gap_init();
  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(new_ble_svc_gatt_defs);

  if (rc != 0) {
    return rc;
  }

  rc = ble_gatts_add_svcs(new_ble_svc_gatt_defs);
  if (rc != 0) {
    return rc;
  }

  return 0;
}

/**
 * @brief 通过BLE通知主动发送数据到已订阅的手机客户端
 * @param data 要发送的数据指针
 * @param len  数据长度（字节）
 * @return esp_err_t 发送结果，ESP_OK表示成功
 */
esp_err_t ble_spp_send_data(const uint8_t* data, size_t len) {
  int rc = 0;
  if (data == NULL || len == 0) {
    return ESP_ERR_INVALID_ARG;
  }

  // 遍历所有连接，向已订阅的客户端发送数据
  for (int conn_handle = 0; conn_handle <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; conn_handle++) {
    if (conn_handle_subs[conn_handle]) {
      // 检查该连接是否已订阅通知
      // 将数据封装到协议栈的 mbuf 中
      struct os_mbuf* txom = ble_hs_mbuf_from_flat(data, len);
      if (txom == NULL) {
        MODLOG_DFLT(ERROR, "Failed to allocate mbuf for sending");
        continue;
      }

      // 关键调用：发送通知
      rc = ble_gatts_notify_custom(conn_handle, ble_spp_svc_gatt_read_val_handle, txom);

      if (rc == 0) {
        MODLOG_DFLT(INFO, "Notification sent successfully to conn_handle=%d", conn_handle);
      }
      else {
        MODLOG_DFLT(ERROR, "Failed to send notification to conn_handle=%d, rc=%d", conn_handle, rc);
        // 释放分配失败时的 mbuf
        os_mbuf_free_chain(txom);
      }
    }
  }
  return (rc == 0) ? ESP_OK : ESP_FAIL;
}

void ble_server_uart_task(void* pvParameters) {
  MODLOG_DFLT(INFO, "BLE server UART_task started\n");
  uint8_t rx_buffer[BLE_RX_ITEM_SIZE];
  for (;;) {
    if (xQueueReceive(spp_common_uart_queue, &rx_buffer, portMAX_DELAY)) {
      MODLOG_DFLT(INFO, "data: %d", rx_buffer[0]);
    }
  }
  vTaskDelete(NULL);
}


void ble_init(void) {
  esp_err_t ret = nimble_port_init();
  if (ret != ESP_OK) {
    MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
    return;
  }

  /* Initialize connection_handle array */
  for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
    conn_handle_subs[i] = false;
  }

  spp_common_uart_queue = xQueueCreate(BLE_RX_QUEUE_LEN, BLE_RX_ITEM_SIZE);
  if (spp_common_uart_queue == NULL) {
    MODLOG_DFLT(ERROR, "Failed to create BLE RX queue!\n");
  }

  xTaskCreate(ble_server_uart_task, "robot_ctrl", 4096, NULL, 8, NULL);

  /* Initialize the NimBLE host configuration. */
  ble_hs_cfg.reset_cb = ble_spp_server_on_reset;
  ble_hs_cfg.sync_cb = ble_spp_server_on_sync;
  ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
#ifdef CONFIG_EXAMPLE_BONDING
  ble_hs_cfg.sm_bonding = 1;
#endif
#ifdef CONFIG_EXAMPLE_MITM
  ble_hs_cfg.sm_mitm = 1;
#endif
#ifdef CONFIG_EXAMPLE_USE_SC
  ble_hs_cfg.sm_sc = 1;
#else
  ble_hs_cfg.sm_sc = 0;
#endif
#ifdef CONFIG_EXAMPLE_BONDING
  ble_hs_cfg.sm_our_key_dist = 1;
  ble_hs_cfg.sm_their_key_dist = 1;
#endif

#if MYNEWT_VAL(BLE_GATTS)
  /* Register custom service */
  int rc = gatt_svr_init();
  assert(rc == 0);

  /* Set the default device name. */
  rc = ble_svc_gap_device_name_set("ROBOT");
  assert(rc == 0);
#endif

  /* XXX Need to have template for store */
  ble_store_config_init();
  nimble_port_freertos_init(ble_spp_server_host_task);
}
