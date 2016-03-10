/*
Copyright (C) 2016 Mustafa Gezen

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This file is going to make it easier for you to work with xcb. Feel free to copy
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xcb_helper.h"

const char *atom_names[] = {
    "_NET_WM_WINDOW_TYPE",
    "_NET_WM_WINDOW_TYPE_DOCK",
    "_NET_WM_DESKTOP",
    "_NET_WM_STRUT_PARTIAL",
    "_NET_WM_STRUT",
    "_NET_WM_STATE",
    "_NET_WM_STATE_STICKY",
    "_NET_WM_STATE_ABOVE",
};

void xcb_h_check_cookie(xcb_helper_struct *internal, xcb_void_cookie_t cookie, const char *err) {
  xcb_generic_error_t *error = xcb_request_check(internal->c, cookie);
  if (error) {
    printf("%s\n", err);
    exit(-1);
    xcb_h_destroy(-1, internal);
  }
}

void xcb_h_atom(xcb_helper_struct *internal) {
    internal->c = xcb_connect(NULL, NULL);

    const int atoms = sizeof(atom_names)/sizeof(char *);
    xcb_intern_atom_cookie_t atom_cookie[atoms];
    xcb_intern_atom_reply_t *atom_reply;

    for (int i = 0; i < atoms; i++)
        atom_cookie[i] = xcb_intern_atom(internal->c, 0, strlen(atom_names[i]), atom_names[i]);

    for (int i = 0; i < atoms; i++) {
        atom_reply = xcb_intern_atom_reply(internal->c, atom_cookie[i], NULL);
        if (!atom_reply)
            return;
        internal->atom_list[i] = atom_reply->atom;
        free(atom_reply);
    }
}

void _xcb_h_setup(xcb_helper_struct *internal) {
    const xcb_setup_t *setup = xcb_get_setup(internal->c);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    internal->screen = screen;
}

void xcb_h_win_setup(xcb_helper_struct *internal) {
    internal->window = xcb_generate_id(internal->c);
    xcb_create_window(internal->c, XCB_COPY_FROM_PARENT, internal->window, internal->screen->root, internal->x, 0, internal->width, internal->height, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, internal->screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        (const uint32_t []){internal->background, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS});
}

void xcb_h_gc_setup(xcb_helper_struct *internal) {
  internal->gc[GC0] = xcb_generate_id(internal->c);
  uint32_t mask = XCB_GC_FOREGROUND;
  uint32_t value[] = { internal->background, 0 };

  xcb_create_gc(internal->c, internal->gc[GC0], internal->window, mask, value);

  internal->gc[GC1] = xcb_generate_id(internal->c);
  xcb_create_gc(internal->c, internal->gc[GC1], internal->window, mask, (uint32_t[]){ internal->rect_border, 0 });

  internal->gc[GC2] = xcb_generate_id(internal->c);
  xcb_create_gc(internal->c, internal->gc[GC2], internal->window, mask, value);
}

void xcb_h_setup(xcb_helper_struct *internal) {
  xcb_h_atom(internal);
  _xcb_h_setup(internal);
  xcb_h_win_setup(internal);
  xcb_h_gc_setup(internal);
}

void xcb_h_map(xcb_helper_struct *internal) {
  xcb_map_window(internal->c, internal->window);
}

void xcb_h_change_property(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, int type) {
  xcb_change_property(internal->c, mode, internal->window, internal->atom_list[wm], atom, m, m1, &internal->atom_list[type]);
}

void xcb_h_change_property_uint32(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, const uint32_t type[]) {
  xcb_change_property(internal->c, mode, internal->window, internal->atom_list[wm], atom, m, m1, type);
}

void xcb_h_change_property_strut(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, int type[12]) {
  xcb_change_property(internal->c, mode, internal->window, internal->atom_list[wm], atom, m, m1, type);
}

void xcb_h_change_wmname(xcb_helper_struct *internal, const char *name, int len) {
  xcb_change_property(internal->c, XCB_PROP_MODE_REPLACE, internal->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, len, name);
}

void xcb_h_change_fontc_gc(xcb_helper_struct *internal, int gcnum, unsigned long color) {
  xcb_change_gc(internal->c, internal->gc[GC_FONT], XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, (const uint32_t[]){ color,
    internal->background, internal->font });
}

void xcb_h_change_color_gc(xcb_helper_struct *internal, int gcnum, unsigned long color) {
  xcb_change_gc(internal->c, internal->gc[gcnum], XCB_GC_FOREGROUND, (const uint32_t[]){ color });
}

void xcb_h_draw_rect(xcb_helper_struct *internal, int gcnum, int num, xcb_rectangle_t rect[]) {
  xcb_poly_rectangle(internal->c, internal->window, internal->gc[gcnum], num, rect);
}

void xcb_h_draw_fill_rect(xcb_helper_struct *internal, int gcnum, int num, xcb_rectangle_t rect[]) {
  xcb_poly_fill_rectangle(internal->c, internal->window, internal->gc[gcnum], num, rect);
}

void xcb_h_setup_font(xcb_helper_struct *internal, const char *fontname) {
  internal->font = xcb_generate_id (internal->c);
  xcb_void_cookie_t font_c = xcb_open_font_checked(internal->c, internal->font, strlen(fontname), fontname);
  xcb_h_check_cookie(internal, font_c, "cant open font");

  internal->gc[GC_FONT] = xcb_generate_id(internal->c);
  xcb_create_gc(internal->c, internal->gc[GC_FONT], internal->window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, (const uint32_t[]){ internal->font_color,
    internal->background, internal->font });
}

int xcb_h_draw_text(xcb_helper_struct *internal, int gcnum, int x, int y, const char *text) {
  if (!internal->font) {
    printf("no font\n");
    return XCB_H_NO_FONT;
  }

  xcb_void_cookie_t fontd_c = xcb_image_text_8_checked(internal->c, strlen(text), internal->window, internal->gc[gcnum], x, y, text);
  xcb_h_check_cookie(internal, fontd_c, "cant draw text\n");

  return 0;
}

void xcb_h_destroy(int exit_status, void *internal) {
  const xcb_helper_struct *_internal = (xcb_helper_struct*)internal;
  for (int i = 0; i < sizeof(_internal->gc); i++) {
    if (_internal->gc[i])
      xcb_free_gc(_internal->c, _internal->gc[i]);
  }

  if (_internal->c)
    xcb_disconnect(_internal->c);

  if (internal)
    free(internal);
}
