// Entropy From Camera Page - Generate mnemonic from camera entropy

#ifndef ENTROPY_FROM_CAMERA_H
#define ENTROPY_FROM_CAMERA_H

#include <lvgl.h>

/**
 * @brief Create the entropy from camera page
 *
 * @param parent Parent LVGL object where the page will be created
 * @param return_cb Callback function to call when returning to previous page
 */
void entropy_from_camera_page_create(lv_obj_t *parent, void (*return_cb)(void));

/**
 * @brief Show the entropy from camera page
 */
void entropy_from_camera_page_show(void);

/**
 * @brief Hide the entropy from camera page
 */
void entropy_from_camera_page_hide(void);

/**
 * @brief Destroy the entropy from camera page and free resources
 */
void entropy_from_camera_page_destroy(void);

/**
 * @brief Get the generated mnemonic
 *
 * @return Generated mnemonic string (caller must free), or NULL if not
 * available
 */
char *entropy_from_camera_get_completed_mnemonic(void);

#endif // ENTROPY_FROM_CAMERA_H
