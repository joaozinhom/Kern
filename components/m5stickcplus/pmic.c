#include "bsp/m5stickcplus.h"
#include "bsp/pmic.h"
#include "driver/adc_oneshot.h"
#include "esp_check.h"

static adc_oneshot_unit_handle_t adc_handle;
static i2c_master_dev_handle_t axp_handle;
static bool pmic_initialized;

static esp_err_t axp_write(uint8_t reg, uint8_t value)
{
    const uint8_t data[] = {reg, value};
    return i2c_master_transmit(axp_handle, data, sizeof(data), 100);
}

static esp_err_t axp_set_bits(uint8_t reg, uint8_t bits)
{
    uint8_t value;
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(axp_handle, &reg, 1, &value, 1, 100), "m5stickcplus", "axp read");
    return axp_write(reg, value | bits);
}

esp_err_t bsp_pmic_init(void)
{
    if (pmic_initialized) {
        return ESP_OK;
    }
#if CONFIG_KERN_BOARD_M5STICKC_PLUS
    ESP_RETURN_ON_ERROR(bsp_i2c_init(), "m5stickcplus", "i2c");
    const i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34,
        .scl_speed_hz = 40000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bsp_i2c_get_handle(), &config, &axp_handle), "m5stickcplus", "axp");
    ESP_RETURN_ON_ERROR(axp_set_bits(0x12, 0x0C), "m5stickcplus", "display power");
    pmic_initialized = true;
    return ESP_OK;
#else
    const adc_oneshot_unit_init_cfg_t unit_config = {.unit_id = ADC_UNIT_1};
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&unit_config, &adc_handle), "m5stickcplus", "adc");
    const adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_2, &channel_config), "m5stickcplus", "battery channel");
    pmic_initialized = true;
    return ESP_OK;
#endif
}

esp_err_t bsp_pmic_power_off(void)
{
#if CONFIG_KERN_BOARD_M5STICKC_PLUS
    return axp_handle ? axp_write(0x32, 0x80) : ESP_ERR_INVALID_STATE;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t bsp_pmic_get_battery_mv(uint16_t *mv)
{
    if (!mv) {
        return ESP_ERR_INVALID_ARG;
    }
#if CONFIG_KERN_BOARD_M5STICKC_PLUS2
    if (!adc_handle) {
        return ESP_ERR_INVALID_STATE;
    }
    int raw = 0;
    ESP_RETURN_ON_ERROR(adc_oneshot_read(adc_handle, ADC_CHANNEL_2, &raw), "m5stickcplus", "battery adc");
    *mv = (uint16_t)(raw * 17 / 10);
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t bsp_pmic_get_battery_percent(uint8_t *pct)
{
    uint16_t mv;
    ESP_RETURN_ON_ERROR(bsp_pmic_get_battery_mv(&mv), "m5stickcplus", "battery voltage");
    if (mv >= 4200) {
        *pct = 100;
    } else if (mv <= 3200) {
        *pct = 0;
    } else {
        *pct = (uint8_t)((mv - 3200) * 100 / 1000);
    }
    return ESP_OK;
}

esp_err_t bsp_pmic_get_charge_status(bsp_pmic_chg_t *status)
{
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }
    *status = BSP_PMIC_CHG_ABSENT;
    return ESP_ERR_NOT_SUPPORTED;
}

bool bsp_pmic_is_vbus_present(void) { return false; }
bool bsp_pmic_is_available(void) { return true; }
bool bsp_pmic_can_power_off(void)
{
#if CONFIG_KERN_BOARD_M5STICKC_PLUS
    return axp_handle != NULL;
#else
    return false;
#endif
}
