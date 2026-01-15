idf_component_register(
    SRCS "src/sd_card.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
    PRIV_REQUIRES fatfs sdmmc vfs
)
