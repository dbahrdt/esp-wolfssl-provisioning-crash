#pragma once

#include "sdkconfig.h"

#include <hap.h>
#include "provisioning.h"

namespace hap {

class HAPManager
{
public:
	const static char * TAG;
public:
	HAPManager(HAPManager const&) = delete;
	void operator=(HAPManager const&) = delete;
	
	static HAPManager& getInstance() {
		static HAPManager instance;
		return instance;
	}
public:
	void init();
	void provision();
private:
	HAPManager() {}
private: //callbacks
	static int identify(hap_acc_t *ha);
	static int write(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv);
private:
	Provisioning _prov;

};

} //end namespace hap
