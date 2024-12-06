idf_component_register(SRCS "MQ-SmokeSensor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_adc OLED freertos LED)
