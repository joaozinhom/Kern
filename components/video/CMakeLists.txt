if(IDF_TARGET STREQUAL "esp32p4")
    idf_component_register(
        SRCS "video.c"
        INCLUDE_DIRS "."
        REQUIRES esp_cam_sensor esp_driver_i2c esp_video esp_sccb_intf wave_4b wave_35 wave_5 wave_43 crowpanel wave_7b secure_memory
    )
else()
    idf_component_register(
        SRCS "video_stub.c"
        INCLUDE_DIRS "."
        REQUIRES esp_driver_i2c secure_memory
    )
endif()
