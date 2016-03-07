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

#include <glib.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "zx.h"
#include "xcb_helper.h"
#include <i3ipc-glib/i3ipc-glib.h>

static int events = 1;
static int redraw = 0;
static int times = 0;

void zx_log_init(zx *internal) {
  const char *homedir;
  if ((homedir = getenv("HOME")) == NULL) {
      homedir = getpwuid(getuid())->pw_dir;
  }

  char logfile[128];
  sprintf(logfile, "%s/.zx.log", homedir);

  internal->log_file = fopen(logfile, "a+");
}

void zx_log(zx *internal, const char *log_) {
  if (internal->log_file) fprintf(internal->log_file, log_);
  printf(log_);
}

void zx_config(xcb_helper_struct *_s) {
    // TODO: Actually parse config here
    _s->x = 0;
    _s->width =1920;
    _s->height = 25;
    _s->background = 0x2C3E50;
}

void xcb_change(xcb_helper_struct *_s) {
    int strut[12] = {0};
    strut[3]  = _s->height;
    strut[10] = _s->x;
    strut[11] = _s->x + _s->width;

    xcb_h_change_property(_s, XCB_PROP_MODE_REPLACE, NET_WM_WINDOW_TYPE, XCB_ATOM_ATOM, 32, 1, NET_WM_WINDOW_TYPE_DOCK);
    xcb_h_change_property(_s, XCB_PROP_MODE_APPEND, NET_WM_STATE, XCB_ATOM_ATOM, 32, 2, NET_WM_STATE_STICKY);
    xcb_h_change_property_uint32(_s, XCB_PROP_MODE_REPLACE, NET_WM_DESKTOP, XCB_ATOM_CARDINAL, 32, 1, (const uint32_t []){ -1 });
    xcb_h_change_property_strut(_s, XCB_PROP_MODE_REPLACE, NET_WM_STRUT_PARTIAL, XCB_ATOM_CARDINAL, 32, 12, strut);
    xcb_h_change_property_strut(_s, XCB_PROP_MODE_REPLACE, NET_WM_STRUT_PARTIAL, XCB_ATOM_CARDINAL, 32, 12, strut);
    xcb_h_change_wmname(_s, "zx", 2);
}

void add_windef(zx *_s, int index, const char *label) {
  _s->windef = realloc(_s->windef, sizeof(zxwin)*_s->windows + 1 + sizeof(zxwin) + 1);
  _s->windef[index] = malloc(sizeof(zxwin) + 1 + sizeof(char*) + sizeof(int)*3);
  _s->windows++;
}

int width_for_rect(zx *_s, xcb_helper_struct *zs) {
  if (_s->windows <= 0)
    return zs->width-3;


  int initial = (zs->width/(_s->windows))-3;

  return initial;
}

void add_win(zx *_s, xcb_helper_struct *zs, const char *label) {
  add_windef(_s, _s->windows, label);

  _s->windef[_s->windows-1]->width = 0;
  _s->windef[_s->windows-1]->x = 0;
  _s->windef[_s->windows-1]->title = (char*)label;
  _s->windef[_s->windows-1]->title_len = strlen(label);

  xcb_rectangle_t win_rect[] = {
    0, 0, 0, zs->height-2
  };

  memcpy(_s->windef[_s->windows-1]->win_rect, win_rect, sizeof(win_rect));
}

void clear_window(xcb_helper_struct *_s, zx *zs) {
  xcb_h_draw_fill_rect(_s, GC0, 1, (xcb_rectangle_t[]){ 0, 0, _s->width, _s->height });
}

void scan_width(zx *_s, xcb_helper_struct *zs) {
  for (int i = 0; i < _s->windows; i++) {
    int width = width_for_rect(_s, zs);
    int x = 2;

    if (width != zs->width-3 && i != 0) {
      x += _s->windef[i-1]->x + width;
    }

    _s->windef[i]->width = width;
    _s->windef[i]->x = x;

    _s->windef[i]->win_rect->x = x;
    _s->windef[i]->win_rect->width = width;

    printf("%d; %d - %d\n", i, width, x);
  }
}

void draw_windows(zx *_s, xcb_helper_struct *zs) {
  for (int i = 0; i < _s->windows; i++) {
    xcb_h_draw_rect(zs, GC1, 1, _s->windef[i]->win_rect);

    xcb_h_change_fontc_gc(zs, GC_FONT, 0xFFFFFF);
    xcb_h_draw_text(zs, GC_FONT, _s->windef[i]->x+_s->windef[i]->width/2-(_s->windef[i]->title_len*2), zs->height-10, _s->windef[i]->title);
  }
}

void clear_windows_main(zx *internal, xcb_helper_struct *ints) {
  for (int i = 0; i < internal->windows; i++) {
    internal->windef[i] = NULL;
  }

  internal->windows = 0;

  if (internal->windef)
    free(internal->windef);

  internal->windef = malloc(sizeof(zxwin) + 1);
}

typedef struct zxinfo {
  zx *internal;
  xcb_helper_struct *s;
} zxinfo;

void add_win_list(i3ipcCon *win, zxinfo *info) {
  gchar *win_name;
  g_object_get(win, "name", &win_name, NULL);

  add_win(info->internal, info->s, win_name);

  g_object_unref(win);
}

void scan_scratchpad(i3ipcCon *win, zxinfo *info) {
  GList *scwin;
  scwin = (GList*)i3ipc_con_get_nodes(win);
  g_list_foreach(scwin, (GFunc)add_win_list, info);
  g_object_unref(win);
}

static void win_callback(GObject *object, i3ipcWorkspaceEvent *event, gpointer user_data) {
  printf("test\n");
  redraw = 1;
}

void draw_wins_main(xcb_helper_struct *_s, zx *zs) {
  clear_windows_main(zs, _s);
  i3ipcConnection *conn;
  i3ipcCon *reply;
  GError *err = NULL;

  conn = i3ipc_connection_new(NULL, NULL);
  reply = i3ipc_connection_get_tree(conn, &err);

  if (err) {
    zx_log(zs, "ERROR! Error calling i3ipc_connection_get_tree!\n");
    events = 0;
    return;
  }

  i3ipcCon *scratchpad;
  scratchpad = i3ipc_con_scratchpad(reply);

  GList *scnode;
  scnode = (GList*)i3ipc_con_get_floating_nodes(scratchpad);

  zxinfo *info = malloc(sizeof(_s) + sizeof(zs) + 1);
  info->s = _s;
  info->internal = zs;

  g_list_foreach(scnode, (GFunc)scan_scratchpad, info);

  i3ipcCommandReply *cr;
  cr = i3ipc_connection_subscribe(conn, I3IPC_EVENT_WORKSPACE, &err);

  if (err || !cr) {
    zx_log(zs, "ERROR! Could not subscribe to I3IPC_EVENT_WINDOW!");
    events = 0;
    return;
  }

  if (!cr->success) {
    char temp[128];
    sprintf(temp, "ERROR! An error occured while subscribing to i3 event: %s", cr->error);
    zx_log(zs, strdup(temp));
    events = 0;
    return;
  }

  g_signal_connect(conn, "workspace", G_CALLBACK(win_callback), NULL);
  printf("signal\n");

  i3ipc_command_reply_free(cr);
  g_object_unref(reply);
  g_object_unref(conn);
  g_object_unref(scratchpad);

  scan_width(zs, _s);
  draw_windows(zs, _s);
}

void sighandle(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    events = 0;
  }
}

int main(int argc, char **argv) {
    struct xcb_helper_struct *_s = malloc(sizeof(xcb_helper_struct) + 1);
    struct zx *zs = malloc(sizeof(zx) + 1 + sizeof(zxwin) + 1);
    zs->windows = 0;

    zx_log_init(zs);

    zx_config(_s);

    on_exit(xcb_h_destroy, (void*)_s);
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    xcb_h_setup(_s);
    xcb_change(_s);

    xcb_h_map(_s);

    xcb_h_setup_font(_s, "fixed", 0xFFFFFF);

    FL(_s);

    XCBPOLL(events)
    XCBEX(_s, zs, draw_wins_main, clear_window, redraw)
    break;
    XCBPE()

    if (zs->windef)
      free(zs->windef);

    free(zs);
    printf("Terminating..\n");

    return 0;
}
