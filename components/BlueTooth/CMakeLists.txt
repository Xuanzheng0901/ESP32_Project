idf_component_register(SRCS "BlueTooth.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt nvs_flash freertos HTTP LED)
