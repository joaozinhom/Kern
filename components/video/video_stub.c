#include "video.h"

esp_err_t app_video_init_once(i2c_master_bus_handle_t i2c_bus_handle)
{
    (void)i2c_bus_handle;
    return ESP_ERR_NOT_SUPPORTED;
}

bool app_video_is_ready(void) { return false; }
bool app_video_is_streaming(void) { return false; }
uint32_t app_video_get_buf_size(void) { return 0; }
esp_err_t app_video_get_resolution(uint32_t *width, uint32_t *height)
{
    (void)width;
    (void)height;
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t app_video_start(app_video_frame_operation_cb_t operation_cb, int core_id)
{
    (void)operation_cb;
    (void)core_id;
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t app_video_stop(void) { return ESP_OK; }
esp_err_t app_video_set_ae_target(uint32_t level)
{
    (void)level;
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t app_video_set_focus(uint32_t position)
{
    (void)position;
    return ESP_ERR_NOT_SUPPORTED;
}
bool app_video_has_focus_motor(void) { return false; }
bool app_video_has_ae_control(void) { return false; }
uint32_t app_video_ppa_snap_crop(uint32_t crop_max, uint32_t target)
{
    return target <= crop_max ? target : crop_max;
}
