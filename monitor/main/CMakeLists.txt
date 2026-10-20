idf_component_register(SRCS "main.c"
                       REQUIRES esp_http_server esp_wifi esp_netif esp_event nvs_flash
                                esp_driver_tsens esp_timer esp_hw_support esp_rom esp_system
                                esp_app_format spi_flash freertos heap
                       EMBED_TXTFILES "index.html")
