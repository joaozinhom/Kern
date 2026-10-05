if(IDF_TARGET STREQUAL "esp32")
    idf_component_register(
        SRCS "m5stickcplus.c" "pmic.c"
        INCLUDE_DIRS "include"
        REQUIRES driver esp_adc esp_lcd esp_driver_ledc bsp_common
    )
else()
    idf_component_register()
endif()
