/*
 * Load Descriptor Storage Page
 *
 * Lists stored descriptors from flash or SD card.
 * Select -> decrypt (if .kef) or load directly (if .txt) -> validate.
 * Inline delete via trash icon on each entry.
 */

#ifndef LOAD_DESCRIPTOR_STORAGE_H
#define LOAD_DESCRIPTOR_STORAGE_H

#include "../core/storage.h"
#include <lvgl.h>

/**
 * @param parent     Parent LVGL object
 * @param return_cb  Callback when returning to previous page
 * @param success_cb Callback on successful descriptor load
 * @param location   Flash or SD card
 */
void load_descriptor_storage_page_create(lv_obj_t *parent,
                                         void (*return_cb)(void),
                                         void (*success_cb)(void),
                                         storage_location_t location);
void load_descriptor_storage_page_show(void);
void load_descriptor_storage_page_hide(void);
void load_descriptor_storage_page_destroy(void);

#endif /* LOAD_DESCRIPTOR_STORAGE_H */
