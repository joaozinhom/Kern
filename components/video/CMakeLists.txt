if(CONFIG_KERN_HAS_CAMERA)
    set(video_source "video.c")
    set(video_requirements esp_cam_sensor esp_video esp_sccb_intf wave_4b wave_35 wave_5 wave_43 crowpanel wave_7b)
else()
    set(video_source "video_stub.c")
    set(video_requirements driver)
endif()

idf_component_register(
    SRCS ${video_source}
    INCLUDE_DIRS "."
    REQUIRES ${video_requirements}
)
