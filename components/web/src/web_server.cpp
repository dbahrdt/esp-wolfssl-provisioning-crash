#include "web_server.h"

#include <esp_log.h>
#include <sys/param.h>
#include <stdexcept>

namespace web {

const char * WebServer::TAG = "WEBSERVER";

WebServer::WebServer() {
#define CRT(__NAME) \
    extern const unsigned char __NAME[]             asm("_binary_" #__NAME "_pem_start"); \
    std::size_t __NAME ## _len;  \
	{ \
		extern const unsigned char __NAME ## _end []    asm("_binary_" #__NAME "_pem_end"); \
		__NAME ## _len = __NAME ## _end - __NAME; \
	}
	
	CRT(cert)
	CRT(prvtkey);
	
    _server_config = HTTPD_SSL_CONFIG_DEFAULT();
    _server_config.cacert_pem = cert;
    _server_config.cacert_len = cert_len;
    _server_config.prvtkey_pem = prvtkey;
    _server_config.prvtkey_len = prvtkey_len;
	_server_config.port_insecure = 8080;
	_server_config.port_secure = 4443;
    _server_config.httpd.uri_match_fn = httpd_uri_match_wildcard;

    _uri.uri = "/test";
    _uri.method = http_method::HTTP_GET;
	_uri.handler = &WebServer::uri_handler;
	_uri.is_websocket = false;
	
};

void WebServer::Start()
{
	if (_server) {
		ESP_LOGI(TAG, "Web server is already running. Doing a restart.");
		Stop();
	}
	
    // Start API
    ESP_LOGI(TAG, "Starting web server");
    esp_err_t ret = httpd_ssl_start(&_server, &_server_config);

    if (ESP_OK != ret) {
        ESP_LOGE(TAG, "Could not start web server: %d", ret);
        throw std::runtime_error("Could not start web server");
    }
	ret = httpd_register_uri_handler(_server, &_uri);
    if (ESP_OK != ret) {
        ESP_LOGE(TAG, "Could not register uri handler: %d", ret);
        throw std::runtime_error("Could not register uri handler");
    }
};

void WebServer::Stop() {
    httpd_ssl_stop(_server);
	_server = nullptr;
};

esp_err_t WebServer::uri_handler(httpd_req_t *r) {
    return httpd_resp_sendstr(r, "ok");
}

}//end namespace web
