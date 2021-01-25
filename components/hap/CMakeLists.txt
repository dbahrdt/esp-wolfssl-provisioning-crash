# Register components
    idf_component_register(SRC_DIRS  "src"
                       INCLUDE_DIRS  "include"
                       REQUIRES web
                       PRIV_REQUIRES wifi_provisioning esp_hap_core esp_hap_platform esp_hap_apple_profiles esp_http_server esp_https_server)
