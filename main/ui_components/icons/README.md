# Icons

Icon fonts generated from [LVGL Font Converter](https://lvgl.io/tools/fontconverter).

## Generating Icons

1. Go to https://lvgl.io/tools/fontconverter
2. Settings:
   - **Name**: `icons_<size>` (e.g., `icons_24`, `icons_36`)
   - **Size**: Desired pixel size
   - **Bpp**: 4 (anti-aliasing)
   - **Font**: Font Awesome 7 Free-Solid-900.otf (or other icon font)
   - **Range**: Unicode codepoints (e.g., `0xF577` for fingerprint)
   - **Fallback**: None
   - **Stride & Align**: 1
   - **Compress**: No
3. Download and save as `icons_<size>.c`

## Adding to Project

1. Place `.c` file in this directory (auto-included by CMakeLists.txt)

2. Create header file `icons_<size>.h`:
```c
#ifndef ICONS_<SIZE>_H
#define ICONS_<SIZE>_H

#include "lvgl.h"

LV_FONT_DECLARE(icons_<size>);

#define ICON_NAME "\xEF\x95\xB7"  // UTF-8 encoded codepoint

#endif
```

3. Use in code:
```c
#include "icons/icons_<size>.h"

lv_obj_t *label = lv_label_create(parent);
lv_label_set_text(label, ICON_NAME);
lv_obj_set_style_text_font(label, &icons_<size>, 0);
```

## UTF-8 Encoding

Convert Unicode codepoint to UTF-8 escape sequence:
- U+F577 -> `\xEF\x95\xB7`

Online converter: https://www.cogsci.ed.ac.uk/~richard/utf-8.cgi

## Font Awesome Icons

Find icons at https://fontawesome.com/icons - use the Unicode value shown for each icon.
