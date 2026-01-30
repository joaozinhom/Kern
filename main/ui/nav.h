#ifndef NAV_H
#define NAV_H

#include <stdbool.h>

#define NAV_MAX_STACK_DEPTH 16

typedef void (*nav_page_create_fn)(void *parent, void *params);
typedef void (*nav_page_show_fn)(void);
typedef void (*nav_page_hide_fn)(void);
typedef void (*nav_page_destroy_fn)(void);

typedef struct {
  const char *id;
  nav_page_create_fn create;
  nav_page_show_fn show;
  nav_page_hide_fn hide;
  nav_page_destroy_fn destroy;
} nav_page_t;

void nav_init(void *root_parent);
void nav_register(const nav_page_t *page);
bool nav_push(const char *page_id, void *params);
bool nav_pop(void);
bool nav_replace(const char *page_id, void *params);
void nav_pop_to_root(void);
const char *nav_current_page_id(void);
int nav_stack_depth(void);

#endif
