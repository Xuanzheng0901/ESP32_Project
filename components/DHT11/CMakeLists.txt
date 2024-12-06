idf_component_register(SRCS "DHT11.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver freertos OLED LED)
