/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "blink.h"
#include "http_request.h"
/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "sc";
static int wifi_statue = 0;
void smartconfig_example_task(void * parm);
static esp_err_t   nvs_read_data_from_flash(wifi_config_t *wifi_config_stored);
static esp_err_t   nvs_write_data_to_flash(wifi_config_t *wifi_config_to_store);
static esp_err_t  wifi_init_sta(void);
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	wifi_mode_t mode;
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		if(wifi_statue == 1)
		{
			xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
		}
		else
		{
			esp_wifi_connect();
		}
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
	wifi_statue = 1;
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
	
}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "SC_STATUS_LINK");
            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
			nvs_write_data_to_flash(wifi_config);
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}

void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
	if(wifi_init_sta()!= ESP_OK)
	{
		initialise_wifi();
	}
	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 8, NULL);
	xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}
static esp_err_t  nvs_write_data_to_flash(wifi_config_t *wifi_config_to_store)
{
    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *DATA1 = "wifi_config";

	ESP_LOGI(TAG,"ssid:%s passwd:%s\r\n", wifi_config_to_store->sta.ssid, wifi_config_to_store->sta.password);
    ESP_LOGI(TAG, "set size:%u\r\n", sizeof(*wifi_config_to_store));
    if(nvs_open(NVS_CUSTOMER, NVS_READWRITE, &handle) != ESP_OK)
		return ESP_FAIL;
    ESP_ERROR_CHECK( nvs_set_blob( handle, DATA1, wifi_config_to_store, sizeof(*wifi_config_to_store)) );

    ESP_ERROR_CHECK( nvs_commit(handle) );
    nvs_close(handle);
	return ESP_OK;
}
static esp_err_t  nvs_read_data_from_flash(wifi_config_t *wifi_config_stored)
{
    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *DATA1 = "wifi_config";
   
    memset(wifi_config_stored, 0x0, sizeof(*wifi_config_stored));
    uint32_t len = sizeof(*wifi_config_stored);

    if(nvs_open(NVS_CUSTOMER, NVS_READWRITE, &handle) != ESP_OK)
		return ESP_FAIL;
    ESP_ERROR_CHECK ( nvs_get_blob(handle, DATA1, wifi_config_stored, &len) );
    ESP_LOGI(TAG,"[wifi_config]: ssid:%s passwd:%s\r\n", wifi_config_stored->sta.ssid, wifi_config_stored->sta.password);

    nvs_close(handle);
	return ESP_OK;
}
static esp_err_t  wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config;
	if(nvs_read_data_from_flash(&wifi_config)!=ESP_OK)
	{
		return ESP_FAIL;
	}
	wifi_statue = 0;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);
	
	return ESP_OK;
}
