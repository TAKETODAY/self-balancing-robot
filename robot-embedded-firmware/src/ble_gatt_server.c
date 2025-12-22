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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "ble_gatt_server.h"
#include "protocol_parser.h"
#include "robot_control.h"

static const char* TAG = "BLE_GATT";

// GATT接口句柄
static uint16_t gatts_if = ESP_GATT_IF_NONE;
static uint16_t conn_id = 0;
static bool is_connected = false;

// 服务与特征值句柄
static uint16_t service_handle;
static esp_gatt_srvc_id_t service_id;
static uint16_t rx_char_handle;
static uint16_t tx_char_handle;
static esp_bt_uuid_t rx_char_uuid = { .len = ESP_UUID_LEN_16, .uuid = { .uuid16 = ROBOT_RX_CHAR_UUID } };
static esp_bt_uuid_t tx_char_uuid = { .len = ESP_UUID_LEN_16, .uuid = { .uuid16 = ROBOT_TX_CHAR_UUID } };

// 协议解析器实例
static protocol_parser_t g_parser;

// GAP事件处理（广播、连接等）
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(TAG, "广播数据设置完成");
      break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
        ESP_LOGI(TAG, "广播启动成功");
      }
      else {
        ESP_LOGE(TAG, "广播启动失败");
      }
      break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      ESP_LOGI(TAG, "广播停止");
      break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
      ESP_LOGI(TAG, "连接参数更新");
      break;

    default:
      break;
  }
}

// GATTS事件处理（核心：数据读写）
static void gatts_event_handler(esp_gatts_cb_event_t event,
  esp_gatt_if_t gatts_if,
  esp_ble_gatts_cb_param_t* param) {

  switch (event) {
    case ESP_GATTS_REG_EVT: {
      // 服务器注册成功
      ESP_LOGI(TAG, "GATTS注册成功，接口号: %d", gatts_if);
      gatts_if = gatts_if;

      // 创建自定义服务
      esp_err_t ret = esp_ble_gatts_create_service(gatts_if, &service_id, 4); // 4个句柄空间
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建服务失败");
      }
      break;
    }

    case ESP_GATTS_CREATE_EVT: {
      // 服务创建成功
      ESP_LOGI(TAG, "服务创建成功，句柄: 0x%04x", param->create.service_handle);
      service_handle = param->create.service_handle;

      // 创建接收指令特征（RX - 写属性）
      esp_attr_control_t attr_control = {
        .auto_rsp = ESP_GATT_AUTO_RSP
      };
      esp_bt_uuid_t rx_uuid = { .len = ESP_UUID_LEN_16, .uuid = { .uuid16 = ROBOT_RX_CHAR_UUID } };

      esp_gatts_attr_db_t rx_attr_db = {
        .attr_control = attr_control,
        .att_desc = {
          .uuid_length = ESP_UUID_LEN_16,
          .uuid_p = (uint8_t*) &rx_uuid.uuid,
          .perm = ESP_GATT_PERM_WRITE_ENCRYPTED, // 需要加密连接才能写
          .max_length = PROTOCOL_MAX_PACKET_SIZE,
          .length = 0,
          .value = NULL
        }
      };

      esp_ble_gatts_add_char_descr(service_handle, &rx_attr_db, ESP_GATT_RSP_BY_APP);

      // 创建发送状态特征（TX - 读/通知属性）
      esp_bt_uuid_t tx_uuid = { .len = ESP_UUID_LEN_16, .uuid = { .uuid16 = ROBOT_TX_CHAR_UUID } };
      esp_gatts_attr_db_t tx_attr_db = {
        .attr_control = attr_control,
        .att_desc = {
          .uuid_length = ESP_UUID_LEN_16,
          .uuid_p = (uint8_t*) &tx_uuid.uuid,
          .perm = ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED,
          .max_length = PROTOCOL_MAX_PACKET_SIZE,
          .length = 0,
          .value = NULL
        }
      };

      esp_ble_gatts_add_char_descr(service_handle, &tx_attr_db, ESP_GATT_RSP_BY_APP);
      break;
    }

    case ESP_GATTS_ADD_CHAR_EVT: {
      // 特征添加成功
      if (param->add_char.attr_handle == rx_char_handle) {
        ESP_LOGI(TAG, "RX特征添加成功，句柄: 0x%04x", rx_char_handle);
      }
      else if (param->add_char.attr_handle == tx_char_handle) {
        ESP_LOGI(TAG, "TX特征添加成功，句柄: 0x%04x", tx_char_handle);
        // 所有特征添加完成，启动服务
        esp_ble_gatts_start_service(service_handle);
      }
      break;
    }

    case ESP_GATTS_START_EVT: {
      // 服务启动成功，开始广播
      ESP_LOGI(TAG, "服务启动成功");
      esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = true,
        .min_interval = 0x20,
        .max_interval = 0x40,
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = sizeof(ROBOT_SERVICE_UUID),
        .p_service_uuid = (uint8_t*) &ROBOT_SERVICE_UUID,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT)
      };

      esp_ble_gap_set_device_name("ESP32_ROBOT");
      esp_ble_gap_config_adv_data(&adv_data);
      break;
    }

    case ESP_GATTS_WRITE_EVT: {
      // 【核心】收到安卓App发来的数据（指令）
      ESP_LOGI(TAG, "收到数据，句柄: 0x%04x，长度: %d",
        param->write.handle, param->write.len);

      if (param->write.handle == rx_char_handle) {
        // 将完整数据包喂给协议解析器
        protocol_parser_feed(&g_parser, param->write.value, param->write.len);

        // 尝试解析完整帧（GATT保证包完整性，通常一次就能解析成功）
        basic_control_frame_t frame;
        if (protocol_parser_try_parse(&g_parser, &frame)) {
          // 解析成功，传递给机器人控制层
          process_control_frame(&frame);
        }
      }
      break;
    }

    case ESP_GATTS_READ_EVT: {
      // 安卓App读取特征值（可返回机器人状态）
      ESP_LOGI(TAG, "读取请求，句柄: 0x%04x", param->read.handle);
      break;
    }

    case ESP_GATTS_CONNECT_EVT: {
      // 客户端连接
      ESP_LOGI(TAG, "客户端已连接，连接ID: %d", param->connect.conn_id);
      conn_id = param->connect.conn_id;
      is_connected = true;

      // 更新连接参数以获得更快的响应
      esp_ble_conn_update_params_t conn_params = {
        .bda = { 0 },
        .min_int = 0x10, // 最小间隔 20ms
        .max_int = 0x20, // 最大间隔 40ms
        .latency = 0, // 从机延迟
        .timeout = 200, // 超时 2秒
      };
      memcpy(conn_params.bda, param->connect.remote_bda, ESP_BD_ADDR_LEN);
      esp_ble_gap_update_conn_params(&conn_params);
      break;
    }

    case ESP_GATTS_DISCONNECT_EVT: {
      // 客户端断开
      ESP_LOGI(TAG, "客户端断开");
      is_connected = false;
      conn_id = 0;

      // 重新开始广播以等待新连接
      esp_ble_gap_start_advertising(&adv_params);
      break;
    }

    case ESP_GATTS_MTU_EVT: {
      // MTU更新事件（影响单包最大长度）
      ESP_LOGI(TAG, "MTU更新: %d", param->mtu.mtu);
      break;
    }

    default:
      break;
  }
}

// 初始化BLE GATT服务器
void ble_gatt_server_init(void) {
  ESP_LOGI(TAG, "初始化BLE GATT服务器");

  // 1. 初始化协议解析器
  protocol_parser_init(&g_parser);

  // 2. 初始化NVS（存储配对信息等）
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // 3. 初始化蓝牙控制器
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)); // 释放经典蓝牙内存
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

  // 4. 初始化Bluedroid
  ESP_ERROR_CHECK(esp_bluedroid_init());
  ESP_ERROR_CHECK(esp_bluedroid_enable());

  // 5. 注册回调
  ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
  ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));

  // 6. 配置MTU和安全性
  ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(PROTOCOL_MAX_PACKET_SIZE));

  // 7. 创建服务
  service_id.is_primary = true;
  service_id.id.inst_id = 0x00;
  service_id.id.uuid.len = ESP_UUID_LEN_16;
  service_id.id.uuid.uuid.uuid16 = ROBOT_SERVICE_UUID;

  ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));

  ESP_LOGI(TAG, "BLE GATT服务器初始化完成，等待连接...");
}

// 通过通知发送数据到安卓App
void ble_send_notification(const uint8_t* data, size_t length) {
  if (!is_connected || length == 0) {
    return;
  }

  // 发送通知（不需要App确认）
  esp_ble_gatts_send_indicate(gatts_if, conn_id, tx_char_handle,
    length, (uint8_t*) data, false);

  // 或者发送指示（需要App确认）
  // esp_ble_gatts_send_indicate(gatts_if, conn_id, tx_char_handle,
  //                             length, (uint8_t *)data, true);
}
