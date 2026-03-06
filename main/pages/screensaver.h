#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <lvgl.h>

typedef void (*screensaver_dismiss_cb_t)(void);

void screensaver_create(lv_obj_t *parent, screensaver_dismiss_cb_t dismiss_cb);
void screensaver_destroy(void);

#endif /* SCREENSAVER_H */
