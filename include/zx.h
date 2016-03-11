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
*/

#include <xcb/xcb.h>
#include <i3ipc-glib/i3ipc-glib.h>

typedef struct zxwin {
  char *title;
  int title_len;
  int x;
  int width;
  xcb_rectangle_t win_rect[4];
  int id;
  i3ipcCon *con;
} zxwin;

typedef struct zx {
  int windows;
  zxwin **windef;
  FILE *log_file;
  i3ipcConnection *conn;
  int active_workspace;
  int floating;
  char *homedir;
  int daemon;
} zx;
