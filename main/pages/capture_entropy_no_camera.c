#include "capture_entropy.h"
#include "../ui/dialog.h"

static void (*return_callback)(void);

void capture_entropy_page_create(lv_obj_t *parent, void (*return_cb)(void))
{
    (void)parent;
    return_callback = return_cb;
    dialog_show_error_timeout("Camera unavailable on this board", return_callback, 0);
}

void capture_entropy_page_show(void) {}
void capture_entropy_page_hide(void) {}
void capture_entropy_page_destroy(void) { return_callback = NULL; }
bool capture_entropy_get_hash(uint8_t *hash_out)
{
    (void)hash_out;
    return false;
}
bool capture_entropy_has_result(void) { return false; }
void capture_entropy_clear(void) {}
