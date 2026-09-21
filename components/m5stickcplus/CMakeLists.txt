if(CONFIG_KERN_BOARD_M5STICKC_PLUS OR CONFIG_KERN_BOARD_M5STICKC_PLUS2)
    idf_component_register(
        SRCS "m5stickcplus.c" "pmic.c"
        INCLUDE_DIRS "include"
        REQUIRES driver esp_lcd esp_driver_ledc bsp_common
    )
else()
    idf_component_register(REQUIRES driver esp_lcd esp_driver_ledc bsp_common)
endif()
