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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <pthread.h>

#include "zx.h"
#include "xcb_helper.h"

static int events = 1;
static int redraw = 0;
static int signal_att = 0;
static GMainLoop *main_loop = NULL;
static pthread_t bgt;

void zx_log_init(zx *internal) {
  char *homedir;
  if ((homedir = getenv("HOME")) == NULL) {
      homedir = getpwuid(getuid())->pw_dir;
  }

  gsize l;
  GError *err = NULL;
  homedir = g_locale_to_utf8(homedir, -1, NULL, &l, &err);

  if (err) {
      fprintf(stderr, "ERROR! Could not convert homedir to UTF-8");
      events = 0;
      return;
  }

  internal->homedir = homedir;

  char logfile[128];
  sprintf(logfile, "%s/.zx.log", homedir);

  internal->log_file = fopen(logfile, "a+");
}

void zx_log(zx *internal, const char *log_) {
  if (internal->log_file) fprintf(internal->log_file, log_);
  fprintf(stderr, log_);
}

void zx_config(xcb_helper_struct *_s, zx *zs) {
    // Const values (Auto defined)
    _s->x = 0;
    _s->width = 0;
    _s->height = 25;

    typedef struct options {
        unsigned long long_value;
        int num_value;
    } options;

    options **_opts = malloc(sizeof(options)+sizeof(int)*4+sizeof(unsigned long)*3+1);
    int count_def = 5;

    for (int i = 0; i < count_def; i++) {
        _opts[i] = malloc(sizeof(int)+sizeof(unsigned long));
    }

    _opts[0]->long_value = 0x2C3E50;
    _opts[1]->long_value = 0x7F8C8D;
    _opts[2]->num_value = 1;
    _opts[3]->num_value = 0;
    _opts[4]->long_value = 0xFFFFFF;

    char config_path[255];
    sprintf(config_path, "%s/.zxconfig", zs->homedir);

    GKeyFile* gkf = g_key_file_new();

    if (!g_key_file_load_from_file(gkf, config_path, G_KEY_FILE_NONE, NULL)){
        fprintf (stderr, "WARNING! Could not read config file %s! No config file present\n", config_path);
        _s->background = _opts[0]->long_value;
        _s->rect_border = _opts[1]->long_value;
        _s->border = _opts[2]->num_value;
        zs->floating = _opts[3]->num_value;
        _s->font_color = _opts[4]->long_value;
        free(_opts);
        return;
    }

    char *opts[] = {"background", "border_color", "border", "floating", "font_color"};
    char *opts_type[] = {"ul", "ul", "int", "int", "ul"};

    GError *err = NULL;

    for (int i = 0; i < count_def; i++) {
        if (strcmp(opts_type[i], "int") == 0) {
            int t = g_key_file_get_integer (gkf, "zx", opts[i], &err);
            if (!err) {
                _opts[i]->num_value = t;
            }
        } else if (strcmp(opts_type[i], "ul") == 0) {
            char *temp = g_key_file_get_value(gkf, "zx", opts[i], &err);
            if (!err) {
                unsigned long t = strtoul(temp, NULL, 0);
                _opts[i]->long_value = t;
            }
        }
        err = NULL;
    }

    _s->background = _opts[0]->long_value;
    _s->rect_border = _opts[1]->long_value;
    _s->border = _opts[2]->num_value;
    zs->floating = _opts[3]->num_value;
    _s->font_color = _opts[4]->long_value;

    if (_s->border == 0) {
        _s->rect_border = _s->background;
    }

    g_key_file_free(gkf);
    free(_opts);
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

void add_windef(zx *_s, int index, const char *label, int id, i3ipcCon *con) {
  _s->windef = realloc(_s->windef, sizeof(zxwin)*_s->windows + 1 + sizeof(zxwin) + 1 + sizeof(label) + sizeof(id) + sizeof(con) + 1);
  _s->windef[index] = malloc(sizeof(zxwin) + 1 + sizeof(char*) + sizeof(int)*3 + sizeof(label) + sizeof(id) + sizeof(con) + 1);
  _s->windows++;
}

int width_for_rect(zx *_s, xcb_helper_struct *zs) {
  if (_s->windows <= 0)
    return zs->width-3;

  int initial = (zs->width/(_s->windows))-3;

  return initial;
}

zxwin *winatx(int x, zx *internal) {
  zxwin *ret = malloc(sizeof(zxwin)+sizeof(i3ipcCon*));

  int found = 0;
  for (int i = 0; i < internal->windows; i++) {
    if (internal->windef[i]->x <= x) {
      ret = internal->windef[i];
      found = 1;
    }
  }

  if (!found) {
    free(ret);
    ret = NULL;
  }

  return ret;
}

void add_win(zx *_s, xcb_helper_struct *zs, const char *label, int id, i3ipcCon *con) {
  add_windef(_s, _s->windows, label, id, con);

  _s->windef[_s->windows-1]->width = 0;
  _s->windef[_s->windows-1]->x = 0;
  _s->windef[_s->windows-1]->title = (char*)label;
  _s->windef[_s->windows-1]->title_len = strlen(label);
  _s->windef[_s->windows-1]->id = id;
  _s->windef[_s->windows-1]->con = con;

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
  }
}

void draw_windows(zx *_s, xcb_helper_struct *zs) {
  for (int i = 0; i < _s->windows; i++) {
    xcb_h_draw_rect(zs, GC1, 1, _s->windef[i]->win_rect);
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

  gint id;
  g_object_get(win, "id", &id, NULL);

  add_win(info->internal, info->s, win_name, id, win);
}

void scan_scratchpad(i3ipcCon *win, zxinfo *info) {
  GList *scwin;
  scwin = (GList*)i3ipc_con_get_nodes(win);
  g_list_foreach(scwin, (GFunc)add_win_list, info);
}

void win_callback(i3ipcConnection *conn, i3ipcWorkspaceEvent *e, gpointer zs) {
  redraw = 1;
}

void draw_wins_main(xcb_helper_struct *_s, zx *zs) {
  clear_windows_main(zs, _s);
  i3ipcCon *reply;
  GError *err = NULL;

  if (!zs->conn) {
    zs->conn = i3ipc_connection_new(NULL, &err);

    if (err) {
      zx_log(zs, "ERROR! Could not make an i3 ipc connection!\n");
      events = 0;
      return;
    }
  }

  reply = i3ipc_connection_get_tree(zs->conn, &err);

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

  g_object_unref(reply);
  g_object_unref(scratchpad);

  scan_width(zs, _s);
  draw_windows(zs, _s);
}

void sighandle(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    events = 0;
  }
}

void *bg_thread(void *arg) {
  const zxinfo *inf = (zxinfo*)arg;
  xcb_helper_struct *_s = inf->s;
  zx *zs = inf->internal;
  while (events) {
    if (redraw) {
      clear_window(_s, zs);
      draw_wins_main(_s, zs);
      FL(_s)
      redraw = 0;
    }
    if ((_s->event = xcb_poll_for_event(_s->c))) {
      switch (_s->event->response_type & ~0x80) {
        case XCB_EXPOSE:
          _s->expose_ev = (xcb_expose_event_t*)_s->event;
          if (_s->expose_ev->count == 0) {
            draw_wins_main(_s, zs);
          }
          FL(_s);
          break;
      case XCB_BUTTON_PRESS:
        _s->press_ev = (xcb_button_press_event_t*)_s->event;
        GError *err = NULL;
        int x = _s->press_ev->event_x;
        zxwin *win = winatx(x, zs);
        if (win) {
          char command[255];
          sprintf(command, "move window to workspace %d;", zs->active_workspace, zs->active_workspace);
          i3ipc_con_command(win->con, command, &err);
          if (err) {
            zx_log(zs, "ERROR! Error moving window to workspace!\n");
            events = 0;
          }

          if (!zs->floating)
            i3ipc_con_command(win->con, "floating toggle", NULL);
        }
        break;
      }

      free(_s->event);
    }

    usleep(1000);
  }

  if (main_loop) {
    g_main_loop_quit(main_loop);
    g_main_loop_unref(main_loop);
  }

  pthread_exit(&bgt);
}

void workspace_callback(i3ipcConnection *conn, i3ipcWorkspaceEvent *e, zx *zs) {
  if (zs->windows == 0) return;
  if (strcmp(e->change, "focus") == 0) {
    // Why doesn't i3ipc_con provide the workspace num?
    GError *err = NULL;
    GSList *workspaces = i3ipc_connection_get_workspaces(zs->conn, &err);

    if (err) {
      zx_log(zs, "ERROR! Error calling i3ipc_connection_get_workspaces!\n");
      events = 0;
      return;
    }

    i3ipcWorkspaceReply *workspace;

    for (GSList *el = workspaces; el; el = el->next) {
      workspace = (i3ipcWorkspaceReply*)el->data;
      if (workspace->focused == TRUE) {
        zs->active_workspace = workspace->num;
        break;
      }
      i3ipc_workspace_reply_free(workspace);
    }
  }
}

int main(int argc, char **argv) {
    struct xcb_helper_struct *_s = malloc(sizeof(xcb_helper_struct) + 1);
    struct zx *zs = malloc(sizeof(zx) + 1 + sizeof(zxwin) + 1);
    zs->windows = 0;
    zs->active_workspace = 1;
    zs->conn = NULL;

    zx_log_init(zs);

    zx_config(_s, zs);

    on_exit(xcb_h_destroy, (void*)_s);
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    if (!events)
        goto done;

    GError *err = NULL;
    zs->conn = i3ipc_connection_new(NULL, &err);

    if (err) {
      zx_log(zs, "ERROR! Could not make an i3 ipc connection!\n");
      goto done;
    }

    GSList *outputs = i3ipc_connection_get_outputs(zs->conn, &err);
    if (err) {
      zx_log(zs, "ERROR! Could not get i3 outputs!\n");
      goto done;
    }

    for (GSList *el = outputs; el; el = el->next) {
      i3ipcOutputReply *output = (i3ipcOutputReply*)el->data;
      // TODO: Add multi monitor support
      if (!_s->width) {
        _s->width = output->rect->width;
      }
      i3ipc_output_reply_free(output);
    }

    xcb_h_setup(_s);

    xcb_change(_s);

    xcb_h_map(_s);

    xcb_h_setup_font(_s, "fixed");

    FL(_s);

    zxinfo *info = malloc(sizeof(_s) + sizeof(zs) + 1);
    info->s = _s;
    info->internal = zs;
    main_loop = g_main_loop_new(NULL, FALSE);
    int perr = pthread_create(&bgt, NULL, bg_thread, (void*)info);

    if(perr) {
      zx_log(zs, "ERROR! pthread_create failed!\n");
    }

    while (!zs->conn);
    i3ipcCommandReply *cr;
    cr = i3ipc_connection_subscribe(zs->conn, I3IPC_EVENT_WINDOW, &err);

    if (err || !cr) {
      zx_log(zs, "ERROR! Could not subscribe to I3IPC_EVENT_WINDOW!\n");
      goto done;
    }

    if (!cr->success) {
      char temp[128];
      sprintf(temp, "ERROR! An error occured while subscribing to i3 event: %s\n", cr->error);
      zx_log(zs, strdup(temp));
      goto done;
    }

    g_signal_connect(zs->conn, "window", G_CALLBACK(win_callback), zs);

    i3ipc_command_reply_free(cr);

    cr = i3ipc_connection_subscribe(zs->conn, I3IPC_EVENT_WORKSPACE, &err);

    if (err || !cr) {
      zx_log(zs, "ERROR! Could not subscribe to I3IPC_EVENT_WORKSPACE!\n");
      goto done;
    }

    if (!cr->success) {
      char temp[128];
      sprintf(temp, "ERROR! An error occured while subscribing to i3 event: %s\n", cr->error);
      zx_log(zs, strdup(temp));
      goto done;
    }

    g_signal_connect(zs->conn, "workspace", G_CALLBACK(workspace_callback), zs);

    i3ipc_command_reply_free(cr);

    g_main_loop_run(main_loop);

    pthread_join(bgt, NULL);

    goto done;

done:
    if (zs->windef)
      free(zs->windef);

    if (zs->conn)
      g_object_unref(zs->conn);

    free(zs);
    printf("Terminating..\n");

    return 0;
}
