// Load Storage Page — list stored mnemonics from flash or SD card

#include "load_storage.h"
#include "../../core/kef.h"
#include "../../core/storage.h"
#include "../../ui/dialog.h"
#include "../../ui/menu.h"
#include "../../ui/theme.h"
#include "../kef_decrypt_page.h"
#include "../key_confirmation.h"
#include <lvgl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DISPLAYED_MNEMONICS 10

static ui_menu_t *storage_menu = NULL;
static lv_obj_t *storage_screen = NULL;
static lv_obj_t *loading_label = NULL;
static lv_timer_t *init_timer = NULL;
static void (*return_callback)(void) = NULL;
static void (*success_callback)(void) = NULL;
static storage_location_t target_location;

/* File listing */
static char **stored_filenames = NULL;
static int allocated_file_count = 0;
static char *display_names[MAX_DISPLAYED_MNEMONICS] = {0};
static int displayed_count = 0;

/* ---------- Cleanup ---------- */

static void cleanup_file_data(void) {
  if (stored_filenames) {
    storage_free_file_list(stored_filenames, allocated_file_count);
    stored_filenames = NULL;
  }
  allocated_file_count = 0;

  for (int i = 0; i < MAX_DISPLAYED_MNEMONICS; i++) {
    free(display_names[i]);
    display_names[i] = NULL;
  }
  displayed_count = 0;
}

/* ---------- Key confirmation callbacks ---------- */

static void return_from_key_confirmation(void) {
  key_confirmation_page_destroy();
  load_storage_page_show();
}

static void success_from_key_confirmation(void) {
  key_confirmation_page_destroy();
  if (success_callback)
    success_callback();
}

/* ---------- Decrypt callbacks ---------- */

static void return_from_kef_decrypt(void) {
  kef_decrypt_page_destroy();
  load_storage_page_show();
}

static void success_from_kef_decrypt(const uint8_t *data, size_t len) {
  key_confirmation_page_create(
      lv_screen_active(), return_from_key_confirmation,
      success_from_key_confirmation, (const char *)data, len);
  key_confirmation_page_show();
  kef_decrypt_page_destroy();
}

static void load_selected(int idx) {
  if (idx < 0 || idx >= displayed_count)
    return;

  uint8_t *envelope = NULL;
  size_t envelope_len = 0;

  esp_err_t ret = storage_load_mnemonic(target_location, stored_filenames[idx],
                                        &envelope, &envelope_len);
  if (ret != ESP_OK) {
    dialog_show_error("Failed to load file", NULL, 0);
    return;
  }

  if (!kef_is_envelope(envelope, envelope_len)) {
    free(envelope);
    dialog_show_error("Invalid encrypted data", NULL, 0);
    return;
  }

  load_storage_page_hide();
  kef_decrypt_page_create(lv_screen_active(), return_from_kef_decrypt,
                          success_from_kef_decrypt, envelope, envelope_len);
  kef_decrypt_page_show();
  free(envelope); /* kef_decrypt_page copies it */
}

/* ---------- Inline delete ---------- */

static int pending_delete_index = -1;

static void build_menu(void);
static void back_cb(void);
static char *get_display_name(storage_location_t loc, const char *filename);

static void inline_delete_refresh_cb(void *user_data) {
  (void)user_data;

  /* Tear down current menu */
  if (storage_menu) {
    ui_menu_destroy(storage_menu);
    storage_menu = NULL;
  }
  cleanup_file_data();

  /* Re-enumerate files */
  char **raw_filenames = NULL;
  int raw_count = 0;
  esp_err_t ret =
      storage_list_mnemonics(target_location, &raw_filenames, &raw_count);

  if (ret != ESP_OK || raw_count == 0) {
    storage_free_file_list(raw_filenames, raw_count);
    const char *loc_name =
        (target_location == STORAGE_FLASH) ? "flash" : "SD card";
    char msg[64];
    snprintf(msg, sizeof(msg), "No mnemonics found on %s", loc_name);
    dialog_show_error(msg, back_cb, 0);
    return;
  }

  stored_filenames = raw_filenames;
  allocated_file_count = raw_count;
  displayed_count = raw_count > MAX_DISPLAYED_MNEMONICS
                        ? MAX_DISPLAYED_MNEMONICS
                        : raw_count;

  for (int i = 0; i < displayed_count; i++)
    display_names[i] = get_display_name(target_location, stored_filenames[i]);

  build_menu();
}

static void inline_delete_confirm_cb(bool confirmed, void *user_data) {
  (void)user_data;
  if (!confirmed)
    return;

  esp_err_t ret = storage_delete_mnemonic(target_location,
                                          stored_filenames[pending_delete_index]);
  if (ret == ESP_OK) {
    if (target_location == STORAGE_FLASH)
      dialog_show_info(
          "Deleted",
          "Mnemonic deleted.\nFor irrecoverable deletion\nuse Wipe Flash.",
          inline_delete_refresh_cb, NULL, DIALOG_STYLE_OVERLAY);
    else
      dialog_show_info("Deleted", "Mnemonic deleted",
                       inline_delete_refresh_cb, NULL, DIALOG_STYLE_OVERLAY);
  } else {
    dialog_show_error("Failed to delete", NULL, 0);
  }
}

static void delete_action_cb(int idx) {
  if (idx < 0 || idx >= displayed_count)
    return;

  pending_delete_index = idx;
  char msg[80];
  snprintf(msg, sizeof(msg), "Delete \"%s\"?",
           display_names[idx] ? display_names[idx] : stored_filenames[idx]);
  dialog_show_confirm(msg, inline_delete_confirm_cb, NULL,
                      DIALOG_STYLE_OVERLAY);
}

/* ---------- Wipe flash ---------- */

static void wipe_complete_cb(void *user_data) {
  (void)user_data;
  back_cb();
}

static lv_obj_t *wipe_progress = NULL;

static void deferred_wipe_cb(lv_timer_t *timer) {
  (void)timer;
  esp_err_t ret = storage_wipe_flash();

  if (wipe_progress) {
    lv_obj_del(wipe_progress);
    wipe_progress = NULL;
  }

  if (ret == ESP_OK) {
    dialog_show_info("Wiped", "Flash storage erased", wipe_complete_cb, NULL,
                     DIALOG_STYLE_OVERLAY);
  } else {
    dialog_show_error("Failed to wipe flash", NULL, 0);
  }
}

static void wipe_flash_confirm_cb(bool confirmed, void *user_data) {
  (void)user_data;
  if (!confirmed)
    return;

  wipe_progress =
      dialog_show_progress("Wiping", "Erasing flash storage...",
                           DIALOG_STYLE_OVERLAY);
  lv_timer_t *t = lv_timer_create(deferred_wipe_cb, 50, NULL);
  lv_timer_set_repeat_count(t, 1);
}

static void wipe_flash_cb(void) {
  dialog_show_confirm(
      "All mnemonics stored in\nflash will be permanently\nerased. Continue?",
      wipe_flash_confirm_cb, NULL, DIALOG_STYLE_OVERLAY);
}

/* ---------- Menu entry callback ---------- */

static void entry_selected_cb(void) {
  int idx = ui_menu_get_selected(storage_menu);
  if (idx < 0 || idx >= displayed_count)
    return;

  load_selected(idx);
}

/* ---------- Back ---------- */

static void back_cb(void) {
  if (return_callback)
    return_callback();
}

/* ---------- Display name from KEF header ---------- */

static char *get_display_name(storage_location_t loc, const char *filename) {
  uint8_t *envelope = NULL;
  size_t envelope_len = 0;

  if (storage_load_mnemonic(loc, filename, &envelope, &envelope_len) != ESP_OK)
    return strdup(filename);

  const uint8_t *id_ptr = NULL;
  size_t id_len = 0;
  if (kef_parse_header(envelope, envelope_len, &id_ptr, &id_len, NULL, NULL) !=
      KEF_OK) {
    free(envelope);
    return strdup(filename);
  }

  size_t copy_len = id_len < 63 ? id_len : 63;
  char *name = malloc(copy_len + 1);
  if (name) {
    memcpy(name, id_ptr, copy_len);
    name[copy_len] = '\0';
  }

  free(envelope);
  return name;
}

/* ---------- Deferred initialization ---------- */

static void build_menu(void) {
  const char *title = (target_location == STORAGE_FLASH) ? "Load from Flash"
                                                         : "Load from SD Card";

  storage_menu = ui_menu_create(storage_screen, title, back_cb);
  if (!storage_menu)
    return;

  for (int i = 0; i < displayed_count; i++) {
    const char *label =
        display_names[i] ? display_names[i] : stored_filenames[i];
    ui_menu_add_entry_with_action(storage_menu, label, entry_selected_cb,
                                  LV_SYMBOL_TRASH, delete_action_cb);
  }

  if (target_location == STORAGE_FLASH) {
    ui_menu_add_entry(storage_menu, "Wipe Flash", wipe_flash_cb);
    int wipe_idx = storage_menu->config.entry_count - 1;
    lv_obj_t *wipe_label =
        lv_obj_get_child(storage_menu->buttons[wipe_idx], 0);
    lv_obj_set_style_text_color(wipe_label, error_color(), 0);
  }

  ui_menu_show(storage_menu);
}

static void deferred_list_cb(lv_timer_t *timer) {
  (void)timer;
  init_timer = NULL;

  /* This may block for several seconds on first SPIFFS mount (formatting) */
  char **raw_filenames = NULL;
  int raw_count = 0;
  esp_err_t ret =
      storage_list_mnemonics(target_location, &raw_filenames, &raw_count);

  /* Remove loading label */
  if (loading_label) {
    lv_obj_del(loading_label);
    loading_label = NULL;
  }

  if (ret != ESP_OK || raw_count == 0) {
    storage_free_file_list(raw_filenames, raw_count);
    const char *loc_name =
        (target_location == STORAGE_FLASH) ? "flash" : "SD card";
    char msg[64];
    snprintf(msg, sizeof(msg), "No mnemonics found on %s", loc_name);
    dialog_show_error(msg, back_cb, 0);
    return;
  }

  stored_filenames = raw_filenames;
  allocated_file_count = raw_count;
  displayed_count = raw_count > MAX_DISPLAYED_MNEMONICS
                        ? MAX_DISPLAYED_MNEMONICS
                        : raw_count;

  /* Get display names from KEF headers */
  for (int i = 0; i < displayed_count; i++)
    display_names[i] = get_display_name(target_location, stored_filenames[i]);

  build_menu();
}

/* ---------- Page lifecycle ---------- */

void load_storage_page_create(lv_obj_t *parent, void (*return_cb)(void),
                              void (*success_cb)(void),
                              storage_location_t location) {
  if (!parent)
    return;

  return_callback = return_cb;
  success_callback = success_cb;
  target_location = location;

  /* Create screen with loading indicator — the actual storage init
     (SPIFFS format on first use) may block for several seconds */
  storage_screen = theme_create_page_container(parent);

  loading_label = lv_label_create(storage_screen);
  lv_label_set_text(loading_label, "Preparing storage...");
  lv_obj_set_style_text_font(loading_label, theme_font_small(), 0);
  lv_obj_set_style_text_color(loading_label, main_color(), 0);
  lv_obj_align(loading_label, LV_ALIGN_CENTER, 0, 0);

  /* Defer the blocking work so LVGL can render the loading screen first */
  init_timer = lv_timer_create(deferred_list_cb, 50, NULL);
  lv_timer_set_repeat_count(init_timer, 1);
}

void load_storage_page_show(void) {
  if (storage_screen)
    lv_obj_clear_flag(storage_screen, LV_OBJ_FLAG_HIDDEN);
  if (storage_menu)
    ui_menu_show(storage_menu);
}

void load_storage_page_hide(void) {
  if (storage_screen)
    lv_obj_add_flag(storage_screen, LV_OBJ_FLAG_HIDDEN);
  if (storage_menu)
    ui_menu_hide(storage_menu);
}

void load_storage_page_destroy(void) {
  if (init_timer) {
    lv_timer_del(init_timer);
    init_timer = NULL;
  }
  if (storage_menu) {
    ui_menu_destroy(storage_menu);
    storage_menu = NULL;
  }
  if (storage_screen) {
    lv_obj_del(storage_screen);
    storage_screen = NULL;
  }
  loading_label = NULL;

  cleanup_file_data();
  return_callback = NULL;
  success_callback = NULL;
  pending_delete_index = -1;
}
