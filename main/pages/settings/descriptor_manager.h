#ifndef DESCRIPTOR_MANAGER_H
#define DESCRIPTOR_MANAGER_H

#include <lvgl.h>

void descriptor_manager_page_create(lv_obj_t *parent, void (*return_cb)(void));
void descriptor_manager_page_show(void);
void descriptor_manager_page_hide(void);
void descriptor_manager_page_destroy(void);

#endif // DESCRIPTOR_MANAGER_H
