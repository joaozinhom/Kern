#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "sdkconfig.h"

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#include "esp_lv_adapter.h"
#include "lvgl.h"
#endif

#define BSP_LCD_H_RES 240
#define BSP_LCD_V_RES 135
#define BSP_LCD_BITS_PER_PIXEL 16
#define BSP_LCD_COLOR_FORMAT_RGB565 1
#define BSP_LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_RGB
#define BSP_LCD_BIGENDIAN 1

#define BSP_LCD_SPI_MOSI GPIO_NUM_15
#define BSP_LCD_SPI_CLK GPIO_NUM_13
#define BSP_LCD_SPI_CS GPIO_NUM_5

#if CONFIG_KERN_BOARD_M5STICKC_PLUS
#define BSP_LCD_DC GPIO_NUM_23
#define BSP_LCD_RST GPIO_NUM_18
#define BSP_LCD_BACKLIGHT GPIO_NUM_NC
#define BSP_BUTTON_A GPIO_NUM_39
#define BSP_BUTTON_B GPIO_NUM_37
#define BSP_I2C_SDA GPIO_NUM_21
#define BSP_I2C_SCL GPIO_NUM_22
#elif CONFIG_KERN_BOARD_M5STICKC_PLUS2
#define BSP_LCD_DC GPIO_NUM_14
#define BSP_LCD_RST GPIO_NUM_12
#define BSP_LCD_BACKLIGHT GPIO_NUM_27
#define BSP_BUTTON_A GPIO_NUM_39
#define BSP_BUTTON_B GPIO_NUM_35
#define BSP_I2C_SDA GPIO_NUM_NC
#define BSP_I2C_SCL GPIO_NUM_NC
#else
#error "Unsupported M5StickC board"
#endif

#define BSP_I2C_NUM 0

esp_err_t bsp_i2c_init(void);
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
lv_display_t *bsp_display_start(void);
bool bsp_display_lock(uint32_t timeout_ms);
void bsp_display_unlock(void);
#endif

esp_err_t bsp_display_brightness_init(void);
esp_err_t bsp_display_brightness_set(int brightness_percent);
esp_err_t bsp_display_backlight_on(void);
esp_err_t bsp_display_backlight_off(void);
