#include "sdkconfig.h"

#include <wifi_provisioning/scheme_softap.h>
#include <hap.h>
#include <hap_platform_httpd.h>
extern "C" {
#include <hap_wac.h>
}

#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include "provisioning.h"
#include "web_server.h"

namespace hap {

const char * Provisioning::TAG = "PROV";
const int Provisioning::WIFI_CONNECTED_EVENT = BIT0;
EventGroupHandle_t Provisioning::wifi_event_group = NULL;

//event handler based on provisioning example (event_handler)
void Provisioning::prov_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
				web::WebServer::getInstance().Start(std::chrono::seconds(5));
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

void Provisioning::init()
{
	
    esp_netif_init();
	
    esp_err_t err = esp_event_loop_create_default();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_loop_create_default() failed: %d", err);
        return;
    }

    wifi_event_group = xEventGroupCreate();

    /* Register event handler for Wi-Fi, IP and Provisioning related events */
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &prov_event_handler, NULL);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_handler_register() failed: %d", err);
        return;
    }

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init() failed: %d", err);
        return;
    }
    else {
        ESP_LOGI(TAG, "esp_wifi_init() successful");
	}

}

void Provisioning::provision()
{

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    esp_err_t err = wifi_prov_mgr_init(config);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "wifi_prov_mgr_init() failed: %d", err);
        return;
    }

    bool provisioned = false;

    err = wifi_prov_mgr_is_provisioned(&provisioned);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "wifi_prov_mgr_is_provisioned() failed: %d", err);
        return;
    } else {
        ESP_LOGI(TAG, "wifi_prov_mgr_is_provisioned() returned %s", provisioned ? "true" : "false");
    }

    err = hap_wifi_is_provisioned(&provisioned);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "hap_wifi_is_provisioned() failed: %d", err);
        return;
    } else {
        ESP_LOGI(TAG, "hap_wifi_is_provisioned() returned %s", provisioned ? "true" : "false");
    }

    if(!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning..");

        esp_netif_create_default_wifi_ap();
        err = esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL);
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "esp_event_handler_register() failed: %d", err);
            return;
        }

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
		auto service_name = "mwe-wolfssl-crash";

        /* What is the security level that we want (0 or 1):
         *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        // Proof of purchase
        const char *pop = "106000115";

        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
         *     - simply ignored when scheme is wifi_prov_scheme_ble
         */
        const char *service_key = NULL;

        wifi_prov_scheme_softap_set_httpd_handle(hap_platform_httpd_get_handle());

        // here an additional callback "custom" provisioning handler could be created

        /* Start provisioning service */
        err = wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key);
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "wifi_prov_mgr_start_provisioning() failed: %d", err);
            return;
        }
    } else {

        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();

        /* Start Wi-Fi station */
        err = esp_wifi_set_mode(WIFI_MODE_STA);
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_set_mode() failed: %d", err);
            return;
        }

        err = esp_wifi_start();
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_start() failed: %d", err);
            return;
        } else {
			web::WebServer::getInstance().Start(std::chrono::seconds(5));
        }
    }
}

} //end namespace hap
