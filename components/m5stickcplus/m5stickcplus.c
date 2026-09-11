#include "bsp/m5stickcplus.h"
#include "bsp/esp-bsp.h"
#include "bsp_err_check.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7789.h"
#include "esp_log.h"

static esp_lcd_panel_io_handle_t io_handle;
static esp_lcd_panel_handle_t panel_handle;
static i2c_master_bus_handle_t i2c_handle;
static lv_indev_t *button_indev;
static lv_group_t *button_group;
static lv_timer_t *button_timer;
static uint32_t pending_key;
static bool pending_pressed;
static bool last_a;
static bool last_b;

static void button_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    data->key = pending_key;
    data->state = pending_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    if (pending_pressed) {
        pending_pressed = false;
    } else {
        pending_key = 0;
    }
}

static void add_clickable_objects(lv_obj_t *parent)
{
    if (!parent || lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }
    if (lv_obj_has_flag(parent, LV_OBJ_FLAG_CLICKABLE) && !lv_obj_has_state(parent, LV_STATE_DISABLED)
        && !lv_obj_get_group(parent)) {
        lv_group_add_obj(button_group, parent);
    }
    const uint32_t count = lv_obj_get_child_count(parent);
    for (uint32_t index = 0; index < count; ++index) {
        add_clickable_objects(lv_obj_get_child(parent, index));
    }
}

static void button_poll(lv_timer_t *timer)
{
    (void)timer;
    add_clickable_objects(lv_screen_active());
    const bool a = gpio_get_level(BSP_BUTTON_A) == 0;
    const bool b = gpio_get_level(BSP_BUTTON_B) == 0;
    if (!pending_key) {
        if (a && !last_a && b) {
            pending_key = LV_KEY_ESC;
            pending_pressed = true;
        } else if (a && !last_a) {
            pending_key = LV_KEY_ENTER;
            pending_pressed = true;
        } else if (b && !last_b) {
            pending_key = LV_KEY_NEXT;
            pending_pressed = true;
        }
    }
    last_a = a;
    last_b = b;
}

esp_err_t bsp_i2c_init(void)
{
#if CONFIG_KERN_BOARD_M5STICKC_PLUS
    if (i2c_handle) {
        return ESP_OK;
    }
    const i2c_master_bus_config_t config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .sda_io_num = BSP_I2C_SDA,
        .scl_io_num = BSP_I2C_SCL,
        .i2c_port = BSP_I2C_NUM,
    };
    return i2c_new_master_bus(&config, &i2c_handle);
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void) { return i2c_handle; }

esp_err_t bsp_display_brightness_init(void)
{
#if CONFIG_KERN_BOARD_M5STICKC_PLUS2
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_9_BIT,
        .timer_num = LEDC_TIMER_3,
        .freq_hz = 256,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    const ledc_channel_config_t channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_7,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_3,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), "m5stickcplus", "backlight timer");
    return ledc_channel_config(&channel);
#else
    return ESP_OK;
#endif
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
#if CONFIG_KERN_BOARD_M5STICKC_PLUS2
    if (brightness_percent < 0) {
        brightness_percent = 0;
    } else if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    const uint32_t duty = (uint32_t)brightness_percent * 511 / 100;
    ESP_RETURN_ON_ERROR(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_7, duty), "m5stickcplus", "backlight duty");
    return ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_7);
#else
    (void)brightness_percent;
    return ESP_OK;
#endif
}

esp_err_t bsp_display_backlight_on(void) { return bsp_display_brightness_set(100); }
esp_err_t bsp_display_backlight_off(void) { return bsp_display_brightness_set(0); }

static esp_err_t display_init(void)
{
    const spi_bus_config_t bus_config = {
        .sclk_io_num = BSP_LCD_SPI_CLK,
        .mosi_io_num = BSP_LCD_SPI_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * 2,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO), "m5stickcplus", "spi bus");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_SPI_CS,
        .pclk_hz = 32000000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle), "m5stickcplus", "panel io");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), "m5stickcplus", "panel");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_handle), "m5stickcplus", "panel reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), "m5stickcplus", "panel init");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(panel_handle, true), "m5stickcplus", "panel invert");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(panel_handle, true), "m5stickcplus", "panel swap");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(panel_handle, true, false), "m5stickcplus", "panel mirror");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(panel_handle, 40, 53), "m5stickcplus", "panel gap");
    return esp_lcd_panel_disp_on_off(panel_handle, true);
}

static void buttons_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = (1ULL << BSP_BUTTON_A) | (1ULL << BSP_BUTTON_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    button_group = lv_group_create();
    lv_group_set_default(button_group);
    button_indev = lv_indev_create();
    lv_indev_set_type(button_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(button_indev, button_read);
    lv_indev_set_group(button_indev, button_group);
    button_timer = lv_timer_create(button_poll, 30, NULL);
    (void)button_timer;
}

lv_display_t *bsp_display_start(void)
{
    const esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    if (esp_lv_adapter_init(&adapter_config) != ESP_OK) {
        return NULL;
    }
    if (bsp_pmic_init() != ESP_OK) {
        return NULL;
    }
    if (bsp_display_brightness_init() != ESP_OK || display_init() != ESP_OK) {
        return NULL;
    }
    const esp_lv_adapter_display_config_t display_config = {
        .panel = panel_handle,
        .panel_io = io_handle,
        .profile = {
            .interface = ESP_LV_ADAPTER_PANEL_IF_OTHER,
            .rotation = ESP_LV_ADAPTER_ROTATE_0,
            .hor_res = BSP_LCD_H_RES,
            .ver_res = BSP_LCD_V_RES,
            .buffer_height = 30,
            .use_psram = false,
            .enable_ppa_accel = false,
            .require_double_buffer = false,
        },
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE,
    };
    lv_display_t *display = esp_lv_adapter_register_display(&display_config);
    if (!display || esp_lv_adapter_start() != ESP_OK) {
        return NULL;
    }
    buttons_init();
    return display;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return esp_lv_adapter_lock(timeout_ms == 0 ? -1 : (int32_t)timeout_ms) == ESP_OK;
}

void bsp_display_unlock(void) { esp_lv_adapter_unlock(); }
