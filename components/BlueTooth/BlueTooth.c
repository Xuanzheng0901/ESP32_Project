#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#include "sdkconfig.h"
#include "HTTP.h"
#include "LED.h"
#include "OLED.h"

#define TAG "BLE_LOG"
#define DEVICE_NAME "ESP32-C6"

char data_buffer[64] = {0};

extern uint8_t Show_Sec;

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
} profile_inst;

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};
//static uint8_t serv_uuid128[16] = {0xef, 0x68, 0x01, 0x00, 0x9b, 0x35, 0x49, 0x33, 0x9b, 0x10, 0x52, 0xff, 0xa9, 0x74, 0x00, 0x42};


static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) 
{
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                 esp_ble_gatts_cb_param_t *param) {
    static uint16_t service_handle = 0;
    //static uint16_t char_handle = 0;

    switch (event) {
    case ESP_GATTS_REG_EVT:
        profile_inst.service_id.is_primary = true;
        profile_inst.service_id.id.inst_id = 0x00;
        profile_inst.service_id.id.uuid.len = ESP_UUID_LEN_16;
        profile_inst.service_id.id.uuid.uuid.uuid16 = 0xFE;
        esp_ble_gatts_create_service(gatts_if, &profile_inst.service_id, 4);

        break;

    case ESP_GATTS_CREATE_EVT:
        service_handle = param->create.service_handle;
        esp_ble_gatts_add_char(service_handle, 
        &((esp_bt_uuid_t)
        {
            .len = ESP_UUID_LEN_16,
            .uuid.uuid16 = 0x00
        }), ESP_GATT_PERM_WRITE | ESP_GATT_PERM_READ,
        ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ,
        NULL,NULL);
        esp_ble_gatts_start_service(service_handle); 
        
        break;

    case ESP_GATTS_WRITE_EVT:
        ESP_LOGI(TAG, "Data received: %.*s", param->write.len, param->write.value);
        strncpy(data_buffer, (char*)param->write.value, param->write.len);
        
        if (param->write.need_rsp) 
        {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id,param->write.trans_id, ESP_GATT_OK, NULL);
            if(strcmp(data_buffer, "@0000") == 0)
                LED_Close();
            else if(strcmp(data_buffer, "@0001") == 0)
                LED_Restart();
            else if(strcmp(data_buffer, "@0002") == 0)
            {
                Show_Sec = 0;
                OLED2_ShowString(1, 9, "    ");
            }
            else if(strcmp(data_buffer, "@0003") == 0)
            {
                Show_Sec = 1;
                OLED2_ShowBigNum(1, 6, 10);
            }
            else if(param->write.len == 6)
                HTTP_Get_Weather((char*)param->write.value);
        }
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    default:
        break;
    }
}

void BT_Init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_ble_gap_set_device_name("ESP32-C6");
    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_start_advertising(&adv_params);
    esp_ble_gatts_register_callback(gatts_event_handler);
    
    esp_ble_gatts_app_register(0);
    esp_ble_gatt_set_local_mtu(500);
    OLED2_ShowIcon(1, 13, 6);
    return;
}