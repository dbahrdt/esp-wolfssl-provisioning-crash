#include "sdkconfig.h"

#include <hap_apple_servs.h>
#include <hap_apple_chars.h>
#include <stdexcept>

#include "esp_log.h"
#include "hap_manager.h"

namespace hap {

const char * HAPManager::TAG = "HAP";

void HAPManager::init() {
	//init based on smart_outlet_thread_entry
	
    if(hap_init(HAP_TRANSPORT_WIFI) != HAP_SUCCESS) {
        ESP_LOGE(TAG, "hap_init() failed");
		throw std::runtime_error("hap_init() failed");
    }

    hap_acc_t *accessory;
    hap_serv_t *service;

    /* Initialise the mandatory parameters for Accessory which will be added as
     * the mandatory services internally
     */
    hap_acc_cfg_t cfg = {
        .name = (char*) "mwe test",
        .model = (char*)"MWE01",
        .manufacturer = (char*)"MWE",
        .serial_num = (char*) "001122334455",
        .fw_rev = (char*) "0.9.0",
        .hw_rev = NULL,
        .pv = (char*) "1.1.0",
        .cid = HAP_CID_OUTLET,
        .identify_routine = &HAPManager::identify,
    };
    /* Create accessory object */
    accessory = hap_acc_create(&cfg);

    /* Add a dummy Product Data */
    uint8_t product_data[] = {'E','S','P','3','2','H','A','P'};
    hap_acc_add_product_data(accessory, product_data, sizeof(product_data));

    /* Create the Outlet Service. Include the "name" since this is a user visible service  */
    service = hap_serv_outlet_create(false, false);
    hap_serv_add_char(service, hap_char_name_create((char*) "My Accessory"));

    /* Set the write callback for the service */
    hap_serv_set_write_cb(service, &HAPManager::write);

    /* Add the Outlet Service to the Accessory Object */
    hap_acc_add_serv(accessory, service);

    /* Add the Accessory to the HomeKit Database */
    hap_add_accessory(accessory);
	#if defined(CONFIG_HAP_MFI_ENABLE)

	if(hap_enable_mfi_auth(HAP_MFI_AUTH_HW) != HAP_SUCCESS) {
		ESP_LOGE(TAG, "hap_enable_mfi_auth() failed");
		throw std::runtime_error("hap_enable_mfi_auth() failed");
	}
	#endif
}

void HAPManager::provision() {
    _prov.init();
	if(HAP_SUCCESS != hap_start()) {
        ESP_LOGE(TAG, "HAP start failed");
        throw std::runtime_error("HAP start failed");
    }
    _prov.provision();
}

int
HAPManager::identify(hap_acc_t *ha) {
	ESP_LOGI(TAG, "Accessory identified");
	return HAP_SUCCESS;
}

int
HAPManager::write(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv) {
	ESP_LOGI(TAG, "Accessory write");
	return HAP_SUCCESS;
}

} //end namespace hap
