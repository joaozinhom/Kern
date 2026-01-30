/*
 * Mnemonic QR Code Backup Page
 * Displays mnemonic as SeedQR format QR code for backup
 */

#ifndef MNEMONIC_QR_H
#define MNEMONIC_QR_H

#include <lvgl.h>

/**
 * Create the mnemonic QR page
 * @param parent Parent LVGL object
 * @param return_cb Callback to call when returning from this page
 */
void mnemonic_qr_page_create(lv_obj_t *parent, void (*return_cb)(void));

/**
 * Show the mnemonic QR page
 */
void mnemonic_qr_page_show(void);

/**
 * Hide the mnemonic QR page
 */
void mnemonic_qr_page_hide(void);

/**
 * Destroy the mnemonic QR page and free resources
 */
void mnemonic_qr_page_destroy(void);

#endif // MNEMONIC_QR_H
