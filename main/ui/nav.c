#include "nav.h"
#include <string.h>

#define NAV_MAX_PAGES 32

typedef struct {
    const nav_page_t *page;
    void *params;
} nav_stack_entry_t;

static const nav_page_t *registered_pages[NAV_MAX_PAGES];
static int num_registered = 0;

static nav_stack_entry_t stack[NAV_MAX_STACK_DEPTH];
static int stack_top = -1;

static void *root_parent = NULL;

static const nav_page_t *find_page(const char *id) {
    for (int i = 0; i < num_registered; i++) {
        if (strcmp(registered_pages[i]->id, id) == 0) {
            return registered_pages[i];
        }
    }
    return NULL;
}

void nav_init(void *parent) {
    root_parent = parent;
    stack_top = -1;
    num_registered = 0;
}

void nav_register(const nav_page_t *page) {
    if (num_registered < NAV_MAX_PAGES && page) {
        registered_pages[num_registered++] = page;
    }
}

bool nav_push(const char *page_id, void *params) {
    if (stack_top >= NAV_MAX_STACK_DEPTH - 1) {
        return false;
    }

    const nav_page_t *page = find_page(page_id);
    if (!page) {
        return false;
    }

    if (stack_top >= 0 && stack[stack_top].page->hide) {
        stack[stack_top].page->hide();
    }

    stack_top++;
    stack[stack_top].page = page;
    stack[stack_top].params = params;

    if (page->create) {
        page->create(root_parent, params);
    }
    if (page->show) {
        page->show();
    }

    return true;
}

bool nav_pop(void) {
    if (stack_top < 0) {
        return false;
    }

    const nav_page_t *current = stack[stack_top].page;
    if (current->hide) {
        current->hide();
    }
    if (current->destroy) {
        current->destroy();
    }

    stack_top--;

    if (stack_top >= 0 && stack[stack_top].page->show) {
        stack[stack_top].page->show();
    }

    return true;
}

bool nav_replace(const char *page_id, void *params) {
    const nav_page_t *page = find_page(page_id);
    if (!page) {
        return false;
    }

    if (stack_top >= 0) {
        const nav_page_t *current = stack[stack_top].page;
        if (current->hide) {
            current->hide();
        }
        if (current->destroy) {
            current->destroy();
        }
    } else {
        stack_top = 0;
    }

    stack[stack_top].page = page;
    stack[stack_top].params = params;

    if (page->create) {
        page->create(root_parent, params);
    }
    if (page->show) {
        page->show();
    }

    return true;
}

void nav_pop_to_root(void) {
    while (stack_top > 0) {
        const nav_page_t *current = stack[stack_top].page;
        if (current->hide) {
            current->hide();
        }
        if (current->destroy) {
            current->destroy();
        }
        stack_top--;
    }

    if (stack_top >= 0 && stack[stack_top].page->show) {
        stack[stack_top].page->show();
    }
}

const char *nav_current_page_id(void) {
    if (stack_top >= 0) {
        return stack[stack_top].page->id;
    }
    return NULL;
}

int nav_stack_depth(void) {
    return stack_top + 1;
}
