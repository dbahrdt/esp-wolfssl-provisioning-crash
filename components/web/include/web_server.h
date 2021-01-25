#pragma once

#include <esp_https_server.h>

namespace web
{
    class WebServer final
    {
	public:
		static const char* TAG;
	public:
        static WebServer & getInstance() {
            static WebServer instance;
            return instance;
        }
    public:
        void Start();
        void Stop();
	public:
		static esp_err_t uri_handler(httpd_req_t *r);
	private:
		WebServer();
		~WebServer() {}
        void Configure();
    private:
        httpd_handle_t _server = nullptr;
        httpd_ssl_config_t _server_config;
		httpd_uri_t _uri;
    };
}
