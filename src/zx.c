/*
    Some parts is taken from bar (https://github.com/lemonboy/bar)
    Copyright (C) 2012 The Lemon Man

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

    Rest:
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
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "zx.h"

void zx_config(zxInternal **internal) {
    // TODO: Actually parse config here
    (*internal)->x = 0;
    (*internal)->width =1920;
    (*internal)->height = 25;
}

void xcb_atom(zxInternal **internal) {
    (*internal)->c = xcb_connect(NULL, NULL);

    const int atoms = sizeof(atom_names)/sizeof(char *);
    xcb_intern_atom_cookie_t atom_cookie[atoms];
    xcb_intern_atom_reply_t *atom_reply;

    for (int i = 0; i < atoms; i++)
        atom_cookie[i] = xcb_intern_atom((*internal)->c, 0, strlen(atom_names[i]), atom_names[i]);

    for (int i = 0; i < atoms; i++) {
        atom_reply = xcb_intern_atom_reply((*internal)->c, atom_cookie[i], NULL);
        if (!atom_reply)
            return;
        (*internal)->atom_list[i] = atom_reply->atom;
        free(atom_reply);
    }
}

void xcb_setup(zxInternal **internal) {
    const xcb_setup_t *setup = xcb_get_setup((*internal)->c);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    (*internal)->screen = screen;
}

void xcb_win_setup(zxInternal **internal) {
    xcb_window_t window = xcb_generate_id((*internal)->c);
    xcb_create_window((*internal)->c, XCB_COPY_FROM_PARENT, window, (*internal)->screen->root, (*internal)->x, 0, (*internal)->width, (*internal)->height, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, (*internal)->screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL,
        (const uint32_t []){0x2C3E50,0x2C3E50,true,XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS});
    (*internal)->window = window;
}

void xcb_map(zxInternal **internal) {
    xcb_map_window((*internal)->c, (*internal)->window);
}

void xcb_change(zxInternal **internal) {
    int strut[12] = {0};
    strut[3]  = (*internal)->height;
    strut[10] = (*internal)->x;
    strut[11] = (*internal)->x + (*internal)->width;

    xcb_change_property((*internal)->c, XCB_PROP_MODE_REPLACE, (*internal)->window, (*internal)->atom_list[NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 32, 1, &(*internal)->atom_list[NET_WM_WINDOW_TYPE_DOCK]);
    xcb_change_property((*internal)->c, XCB_PROP_MODE_APPEND,  (*internal)->window, (*internal)->atom_list[NET_WM_STATE], XCB_ATOM_ATOM, 32, 2, &(*internal)->atom_list[NET_WM_STATE_STICKY]);
    xcb_change_property((*internal)->c, XCB_PROP_MODE_REPLACE, (*internal)->window, (*internal)->atom_list[NET_WM_DESKTOP], XCB_ATOM_CARDINAL, 32, 1, (const uint32_t []){ -1 } );
    xcb_change_property((*internal)->c, XCB_PROP_MODE_REPLACE, (*internal)->window, (*internal)->atom_list[NET_WM_STRUT_PARTIAL], XCB_ATOM_CARDINAL, 32, 12, strut);
    xcb_change_property((*internal)->c, XCB_PROP_MODE_REPLACE, (*internal)->window, (*internal)->atom_list[NET_WM_STRUT], XCB_ATOM_CARDINAL, 32, 4, strut);
    xcb_change_property((*internal)->c, XCB_PROP_MODE_REPLACE, (*internal)->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, 3, "zx");
}

void zx_destroy(zxInternal **internal) {
    xcb_disconnect((*internal)->c);
}

int main(int argc, char **argv) {
    struct zxInternal *_s = malloc(sizeof(zxInternal) + 1);
    xcb_atom(&_s);
    xcb_setup(&_s);
    zx_config(&_s);
    xcb_win_setup(&_s);
    xcb_change(&_s);

    xcb_map(&_s);

    xcb_flush(_s->c);
    pause();
    zx_destroy(&_s);

    return 0;
}
