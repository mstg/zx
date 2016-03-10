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
#include <xcb/xcb.h>

enum {
    NET_WM_WINDOW_TYPE,
    NET_WM_WINDOW_TYPE_DOCK,
    NET_WM_DESKTOP,
    NET_WM_STRUT_PARTIAL,
    NET_WM_STRUT,
    NET_WM_STATE,
    NET_WM_STATE_STICKY,
    NET_WM_STATE_ABOVE,
};

enum {
  GC0,
  GC1,
  GC2,
  GC_FONT
};

enum {
  XCB_H_NO_FONT
};

typedef struct xcb_helper_struct {
  xcb_connection_t *c;
  xcb_window_t window;
  xcb_atom_t atom_list[10];
  xcb_screen_t *screen;
  xcb_gcontext_t gc[4];
  xcb_generic_event_t *event;
  xcb_expose_event_t *expose_ev;
  xcb_button_press_event_t *press_ev;
  xcb_font_t font;
  int count;
  int x;
  int width;
  int height;
  unsigned long background;
  unsigned long rect_border;
  unsigned long font_color;
  int border;
} xcb_helper_struct;

void xcb_h_setup(xcb_helper_struct *internal);

void xcb_h_map(xcb_helper_struct *internal);

void xcb_h_gc_setup(xcb_helper_struct *internal);

void xcb_h_change_property(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, int type);

void xcb_h_change_property_uint32(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, const uint32_t type[]);

void xcb_h_change_property_strut(xcb_helper_struct *internal, int mode, int wm, int atom, int m, int m1, int type[12]);

void xcb_h_change_wmname(xcb_helper_struct *internal, const char *name, int len);

void xcb_h_change_fontc_gc(xcb_helper_struct *internal, int gcnum, unsigned long color);

void xcb_h_change_color_gc(xcb_helper_struct *internal, int gcnum, unsigned long color);

void xcb_h_draw_rect(xcb_helper_struct *internal, int gcnum, int num, xcb_rectangle_t rect[]);

void xcb_h_draw_fill_rect(xcb_helper_struct *internal, int gcnum, int num, xcb_rectangle_t rect[]);

void xcb_h_setup_font(xcb_helper_struct *internal, const char *fontname);

int xcb_h_draw_text(xcb_helper_struct *internal, int gcnum, int x, int y, const char *text);

void xcb_h_destroy(int exit_status, void *internal);

#define FL(internal) \
  xcb_flush(internal->c);
