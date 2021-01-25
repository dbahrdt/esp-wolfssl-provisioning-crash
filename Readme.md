# "Minimum" working example for WolfSSL crash after provisioning
This is mostly based on the provisioning example code.

Steps to reproduce
1. Build and flash to esp32 with mfi chip
2. Start
3. Provision device (SSID: mwe-wolfssl-crash)
4. Call API (curl -k https://your-ip/test)
5. Crash

Crash is likely related with RSA hardware acceleration.
Enable the following in components/esp-tls/include/user_settings.h
```C++
#define NO_WOLFSSL_ESP32WROOM32_CRYPT_RSA_PRI
```
And the crash is gone.

# Generate factory nvs

$HOMEKIT_PATH/tools/factory_nvs_gen/factory_nvs_gen.py 12345678 70SX hap_12345678_70SX
