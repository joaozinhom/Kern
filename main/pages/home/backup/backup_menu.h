/*
 * Backup Menu Page
 * Menu for selecting backup method (Words, QR Code)
 */

#ifndef BACKUP_MENU_H
#define BACKUP_MENU_H

#include <lvgl.h>

/**
 * Create the backup menu page
 * @param parent Parent LVGL object
 * @param return_cb Callback to call when returning from this page
 */
void backup_menu_page_create(lv_obj_t *parent, void (*return_cb)(void));

/**
 * Show the backup menu page
 */
void backup_menu_page_show(void);

/**
 * Hide the backup menu page
 */
void backup_menu_page_hide(void);

/**
 * Destroy the backup menu page and free resources
 */
void backup_menu_page_destroy(void);

#endif // BACKUP_MENU_H
