/*
 * Store Mnemonic Page
 *
 * Encrypts the loaded mnemonic with KEF and saves it to flash or SD card.
 * Flow: ID prompt -> password -> encrypt on CPU1 -> save.
 */

#ifndef STORE_MNEMONIC_H
#define STORE_MNEMONIC_H

#include "../core/storage.h"
#include <lvgl.h>

void store_mnemonic_page_create(lv_obj_t *parent, void (*return_cb)(void),
                                storage_location_t location);
void store_mnemonic_page_show(void);
void store_mnemonic_page_hide(void);
void store_mnemonic_page_destroy(void);

#endif /* STORE_MNEMONIC_H */
