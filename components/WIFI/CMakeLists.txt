idf_component_register(SRCS "WIFI.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_wifi freertos esp_system esp_event lwip OLED HTTP) 
