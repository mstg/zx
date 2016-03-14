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
    xcb_h_destroy(-1, internal);
    exit(-1);
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
    xcb_create_window(internal->c, XCB_COPY_FROM_PARENT, internal->window, internal->screen->root, internal->x, internal->y, internal->width, internal->height, 0,
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
  internal->mapped = 1;
}

void xcb_h_unmap(xcb_helper_struct *internal) {
  xcb_unmap_window(internal->c, internal->window);
  internal->mapped = 0;
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

/*
  From: https://github.com/s-ol/i3bgbar/blob/master/libi3
  Copyright © 2009-2011, Michael Stapelberg and contributors
  All rights reserved.
*/
PangoLayout *create_layout_with_dpi(xcb_helper_struct *_s, cairo_t *cr) {
  PangoLayout *layout;
  PangoContext *context;

  context = pango_cairo_create_context(cr);
  const double dpi = (double)180;
  pango_cairo_context_set_resolution(context, dpi);
  layout = pango_layout_new(context);
  g_object_unref(context);

  return layout;
}

xcb_visualtype_t *get_visualtype(xcb_screen_t *screen) {
  xcb_depth_iterator_t depth_iter;
  for (depth_iter = xcb_screen_allowed_depths_iterator(screen);
  depth_iter.rem;
  xcb_depth_next(&depth_iter)) {
    xcb_visualtype_iterator_t visual_iter;
    for (visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
    visual_iter.rem;
    xcb_visualtype_next(&visual_iter)) {
      if (screen->root_visual == visual_iter.data->visual_id)
      return visual_iter.data;
    }
  }
  return NULL;
}
int predict_text_width_pango(xcb_helper_struct *internal, const char *text) {
    cairo_surface_t *surface = cairo_xcb_surface_create(internal->c, internal->screen->root, internal->root_visual_type, 1, 1);
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = create_layout_with_dpi(internal, cr);

    gint width;
    pango_layout_set_font_description(layout, internal->pfont->pango_desc);
    pango_layout_set_text(layout, text, strlen(text));
    pango_cairo_update_layout(cr, layout);
    pango_layout_get_pixel_size(layout, &width, NULL);

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return width;
}
void draw_text_pango(xcb_helper_struct *internal, const char *text, int x, int y, int max_width) {
    cairo_surface_t *surface = cairo_xcb_surface_create(internal->c, internal->window,
      internal->root_visual_type, internal->x + internal->width, internal->y + internal->height);
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = create_layout_with_dpi(internal, cr);
    gint height;

    int font_width = predict_text_width_pango(internal, text);
    int font_size = pango_font_description_get_size(internal->pfont->pango_desc) / PANGO_SCALE;
    pango_layout_set_font_description(layout, internal->pfont->pango_desc);
    pango_layout_set_width(layout, (max_width) * font_size * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

    pango_layout_set_text(layout, text, strlen(text));

    cairo_set_source_rgb(cr, internal->pfont->pango_font_red, internal->pfont->pango_font_green, internal->pfont->pango_font_blue);
    pango_cairo_update_layout(cr, layout);
    pango_layout_get_pixel_size(layout, NULL, &height);
    cairo_move_to(cr, x - font_width/2, y-internal->font_height);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
/**/

void xcb_h_setup_font(xcb_helper_struct *internal, const char *fontname) {
  if (internal->font_type == PANGO) {
    internal->pfont->pango_desc = pango_font_description_from_string(fontname);
    if (!internal->pfont->pango_desc) {
      fprintf(stderr, "ERROR! Could not load pango font!\n");
      xcb_h_destroy(-1, internal);
      exit(-1);
    }

    internal->root_visual_type = get_visualtype(internal->screen);

    cairo_surface_t *surface = cairo_xcb_surface_create(internal->c, internal->screen->root, internal->root_visual_type, 1, 1);
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = create_layout_with_dpi(internal, cr);
    pango_layout_set_font_description(layout, internal->pfont->pango_desc);

    gint height;
    pango_layout_get_pixel_size(layout, NULL, &height);
    internal->font_height = height;

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return;
  }

  internal->font = xcb_generate_id (internal->c);
  xcb_void_cookie_t font_c = xcb_open_font_checked(internal->c, internal->font, strlen(fontname), fontname);
  xcb_h_check_cookie(internal, font_c, "cant open font");

  xcb_query_font_cookie_t queryreq = xcb_query_font(internal->c, internal->font);
  xcb_query_font_reply_t *font_info = xcb_query_font_reply(internal->c, queryreq, NULL);

  internal->font_width = font_info->max_bounds.character_width;
  internal->font_height = font_info->font_ascent + font_info->font_descent;
  internal->font_descent = font_info->font_descent;

  internal->gc[GC_FONT] = xcb_generate_id(internal->c);
  xcb_create_gc(internal->c, internal->gc[GC_FONT], internal->window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, (const uint32_t[]){ internal->font_color,
    internal->background, internal->font });

  free(font_info);
}

int xcb_h_draw_text(xcb_helper_struct *internal, int gcnum, int x, int y, const char *text, int max_width) {
  if ((internal->font_type == xcb && !internal->font) || (internal->font_type == PANGO && !internal->pfont)) {
    printf("no font\n");
    return XCB_H_NO_FONT;
  }

  xcb_void_cookie_t fontd_c;
  int ch_width;
  switch (internal->font_type) {
    case PANGO:
      draw_text_pango(internal, text, x+max_width/2, y, max_width);
      break;
    case xcb:
      fontd_c = xcb_image_text_8_checked(internal->c, strlen(text), internal->window, internal->gc[gcnum], x + max_width/2 - strlen(text)*2 - 30, y, text);
      xcb_h_check_cookie(internal, fontd_c, "cant draw text\n");
      break;
  }

  return 0;
}

void xcb_h_destroy(int exit_status, void *internal) {
  const xcb_helper_struct *_internal = (xcb_helper_struct*)internal;
  for (int i = 0; i < sizeof(_internal->gc); i++) {
    if (_internal->gc[i])
      xcb_free_gc(_internal->c, _internal->gc[i]);
  }

  switch (_internal->font_type) {
    case PANGO:
      if (_internal->pfont)
        pango_font_description_free(_internal->pfont->pango_desc);
      break;
    case xcb:
      if (_internal->font)
        xcb_close_font(_internal->c, _internal->font);
  }

  if (_internal->c)
    xcb_disconnect(_internal->c);

  if (internal)
    free(internal);
}
