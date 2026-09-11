#include "scanner.h"
#include "../ui/dialog.h"

static void (*return_callback)(void);

void qr_scanner_page_create(lv_obj_t *parent, void (*return_cb)(void))
{
    (void)parent;
    return_callback = return_cb;
    dialog_show_error_timeout("Camera unavailable on this board", return_callback, 0);
}

void qr_scanner_page_show(void) {}
void qr_scanner_page_hide(void) {}
void qr_scanner_page_destroy(void) { return_callback = NULL; }
char *qr_scanner_get_completed_content(void) { return NULL; }
char *qr_scanner_get_completed_content_with_len(size_t *content_len)
{
    if (content_len) {
        *content_len = 0;
    }
    return NULL;
}
bool qr_scanner_is_ready(void) { return false; }
bool qr_scanner_has_completed_result(void) { return false; }
int qr_scanner_get_format(void) { return -1; }
char qr_scanner_get_bbqr_file_type(void) { return 0; }
bool qr_scanner_get_ur_result(const char **ur_type_out, const uint8_t **cbor_data_out, size_t *cbor_len_out)
{
    if (ur_type_out) {
        *ur_type_out = NULL;
    }
    if (cbor_data_out) {
        *cbor_data_out = NULL;
    }
    if (cbor_len_out) {
        *cbor_len_out = 0;
    }
    return false;
}
