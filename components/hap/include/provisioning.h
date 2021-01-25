#pragma once

#include "sdkconfig.h"

#include <freertos/event_groups.h>


namespace hap {

class Provisioning {
public:
	static const char *TAG;
public:
	Provisioning() {}
public:
	void init();
	void provision();
private:
	static void prov_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);
private: //static variables for prov_event_handler (close to the C version)
	static EventGroupHandle_t wifi_event_group;
	static const int WIFI_CONNECTED_EVENT;
};

} //end namespace hap
