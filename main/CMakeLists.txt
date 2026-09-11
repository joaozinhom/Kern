file(GLOB_RECURSE SOURCES "*.c")
list(FILTER SOURCES EXCLUDE REGEX ".*/core/test/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/qr/test/.*")

# Select touch driver based on board
if(CONFIG_KERN_BOARD_WAVE_4B)
    set(TOUCH_DRIVER esp_lcd_touch_gt911)
    set(FONT_POLICY_WIDTH 720)
    set(FONT_POLICY_HEIGHT 720)
elseif(CONFIG_KERN_BOARD_WAVE_35)
    set(TOUCH_DRIVER esp_lcd_touch_ft5x06)
    set(FONT_POLICY_WIDTH 320)
    set(FONT_POLICY_HEIGHT 480)
elseif(CONFIG_KERN_BOARD_WAVE_5)
    set(TOUCH_DRIVER esp_lcd_touch_gt911)
    set(FONT_POLICY_WIDTH 720)
    set(FONT_POLICY_HEIGHT 1280)
elseif(CONFIG_KERN_BOARD_WAVE_43)
    set(TOUCH_DRIVER esp_lcd_touch_gt911)
    set(FONT_POLICY_WIDTH 480)
    set(FONT_POLICY_HEIGHT 800)
elseif(CONFIG_KERN_BOARD_CROWPANEL)
    set(TOUCH_DRIVER esp_lcd_touch_gt911)
    set(FONT_POLICY_WIDTH 1024)
    set(FONT_POLICY_HEIGHT 600)
elseif(CONFIG_KERN_BOARD_WAVE_7B)
    set(TOUCH_DRIVER esp_lcd_touch_gt911)
    set(FONT_POLICY_WIDTH 1024)
    set(FONT_POLICY_HEIGHT 600)
    set(BSP_COMPONENT wave_4b)
elseif(CONFIG_KERN_BOARD_M5STICKC_PLUS OR CONFIG_KERN_BOARD_M5STICKC_PLUS2)
    set(BSP_COMPONENT m5stickcplus)
    set(FONT_POLICY_WIDTH 240)
    set(FONT_POLICY_HEIGHT 135)
    list(FILTER SOURCES EXCLUDE REGEX ".*/qr/scanner\\.c$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/pages/capture_entropy\\.c$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/pages/dev_tools/snapshot\\.c$")
endif()

if(NOT DEFINED BSP_COMPONENT)
    if(CONFIG_KERN_BOARD_WAVE_4B)
        set(BSP_COMPONENT wave_4b)
    elseif(CONFIG_KERN_BOARD_WAVE_35)
        set(BSP_COMPONENT wave_35)
    elseif(CONFIG_KERN_BOARD_WAVE_5)
        set(BSP_COMPONENT wave_5)
    elseif(CONFIG_KERN_BOARD_WAVE_43)
        set(BSP_COMPONENT wave_43)
    elseif(CONFIG_KERN_BOARD_CROWPANEL)
        set(BSP_COMPONENT crowpanel)
    endif()
endif()

if(CONFIG_KERN_HAS_CAMERA)
    set(CAMERA_REQUIREMENTS k_quirc esp_driver_ppa)
endif()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS .
    PRIV_REQUIRES lvgl ${TOUCH_DRIVER} ${BSP_COMPONENT} ${CAMERA_REQUIREMENTS} esp_timer
                  libwally-core cUR sd_card bbqr deflate_codec video spiffs
                  nvs_flash efuse esp_hw_support esp_app_format mbedtls
                  driver app_update bootloader_support
)

# Ignoring a warn_unused_result return is a build error, not a warning. The
# k_quirc and cUR entry points carry the attribute and report failure that way
# and nothing else.
target_compile_options(${COMPONENT_LIB} PRIVATE -Werror=unused-result)

if(DEFINED FONT_POLICY_WIDTH AND DEFINED FONT_POLICY_HEIGHT)
    find_program(PYTHON3_EXECUTABLE python3 REQUIRED)
    execute_process(
        COMMAND ${PYTHON3_EXECUTABLE}
                ${CMAKE_CURRENT_LIST_DIR}/../tools/ui_font_policy.py
                --width ${FONT_POLICY_WIDTH}
                --height ${FONT_POLICY_HEIGHT}
                --emit cmake-icon-defs
        OUTPUT_VARIABLE ICON_FONT_DEFINITIONS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    if(ICON_FONT_DEFINITIONS)
        target_compile_definitions(${COMPONENT_LIB} PRIVATE ${ICON_FONT_DEFINITIONS})
    endif()
endif()
