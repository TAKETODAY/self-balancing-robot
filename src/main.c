/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "battery.h"
#include "wifi.h"
#include "esp_private/log_level.h"

/**
 * Declare the symbol pointing to the former implementation of esp_restart function
 */
// extern void __real_esp_restart(void);

/**
 * Redefine esp_restart function to print a message before actually restarting
 */
// void __wrap_esp_restart(void) {
//   printf("Restarting in progress...\n");
//   /* Call the former implementation to actually restart the board */
//   __real_esp_restart();
// }


void app_main(void) {
  esp_log_set_default_level(ESP_LOG_DEBUG);

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init();

  battery_init();
}
