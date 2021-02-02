# Register components
idf_component_register(SRC_DIRS       "src"
                       INCLUDE_DIRS   "include"
                       PRIV_REQUIRES  esp_https_server
                       EMBED_TXTFILES
                                        "certs/ca.pem"
										"certs/cert.pem"
										"certs/fullchainwolfssl.pem"
										"certs/fullchainopenssl.pem"
										"certs/certselfsign.pem"
                                        "certs/prvtkey.pem"
                       )
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
