#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#include "hap_manager.h"
#include "web_server.h"

static const char* TAG = "MWE_WOLFSSL_CRASH";

extern "C" void app_main(void)
{
    // Setting Log Level to Debug
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "mwe started");

	esp_timer_init();

	auto & hap = hap::HAPManager::getInstance();
	
	hap.init();
	hap.provision();

    // Start main loop
    while(true) {
        ESP_LOGI(TAG, "MWE still running");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
