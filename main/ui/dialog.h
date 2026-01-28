#ifndef DIALOG_H
#define DIALOG_H

#include <stdbool.h>

typedef enum {
    DIALOG_INFO,
    DIALOG_ERROR,
    DIALOG_CONFIRM,
} dialog_type_t;

typedef enum {
    DIALOG_STYLE_FULLSCREEN,
    DIALOG_STYLE_OVERLAY,
} dialog_style_t;

typedef void (*dialog_callback_t)(void *user_data);
typedef void (*dialog_simple_callback_t)(void);
typedef void (*dialog_confirm_callback_t)(bool confirmed, void *user_data);

void dialog_show_info(const char *title, const char *message,
                      dialog_callback_t callback, void *user_data,
                      dialog_style_t style);

void dialog_show_error(const char *message, dialog_simple_callback_t callback,
                       int timeout_ms);

void dialog_show_confirm(const char *message,
                         dialog_confirm_callback_t callback, void *user_data,
                         dialog_style_t style);

void dialog_show_message(const char *title, const char *message);

#endif
