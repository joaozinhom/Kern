#include "snapshot.h"
#include "../../ui/dialog.h"

static void (*return_callback)(void);

void snapshot_page_create(lv_obj_t *parent, void (*return_cb)(void))
{
    (void)parent;
    return_callback = return_cb;
    dialog_show_error_timeout("Camera unavailable on this board", return_callback, 0);
}

void snapshot_page_show(void) {}
void snapshot_page_hide(void) {}
void snapshot_page_destroy(void) { return_callback = NULL; }
