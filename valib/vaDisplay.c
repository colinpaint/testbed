//{{{  includes
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <va/va.h>

#include "va_display.h"

extern const VADisplayHooks va_display_hooks_wayland;
extern const VADisplayHooks va_display_hooks_x11;
extern const VADisplayHooks va_display_hooks_drm;

static const VADisplayHooks *g_display_hooks;
//}}}

static const VADisplayHooks* g_display_hooks_available[] = {
  &va_display_hooks_wayland,
  &va_display_hooks_drm,
  &va_display_hooks_x11,
  NULL
  };

static const char* g_display_name;
const char* g_drm_device_name;

//{{{
static const char* get_display_name (int argc, char* argv[]) {

  const char* display_name = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--display") != 0)
      continue;
    argv[i] = NULL;

    if (++i < argc) {
      display_name = argv[i];
      argv[i] = NULL;
      }
    }

  return display_name;
  }
//}}}
//{{{
static const char* get_drm_device_name (int argc, char* argv[]) {

  const char* device_name = NULL;
  for (int i = 1; i < argc; i++) {
    if (argv[i] && (strcmp(argv[i], "--device") != 0))
      continue;
    argv[i] = NULL;

    if (++i < argc) {
      device_name = argv[i];
      argv[i] = NULL;
      }
    }

  return device_name;
  }
//}}}

//{{{
static void print_display_names() {

  const VADisplayHooks** h;

  printf ("Available displays:\n");
  for (h = g_display_hooks_available; *h != NULL; h++)
    printf ("  %s\n", (*h)->name);
  }
//}}}
//{{{
static void sanitize_args (int* argc, char* argv[]) {

  char** out_args = argv;

  int n = *argc;
  for (int i = 0; i < n; i++) {
    if (argv[i])
      *out_args++ = argv[i];
    }

  *out_args = NULL;
  *argc = out_args - argv;
  }
//}}}

//{{{
void va_init_display_args (int* argc, char* argv[]) {

  const char *display_name;

  display_name = get_display_name (*argc, argv);
  if (display_name && strcmp (display_name, "help") == 0) {
    print_display_names();
    exit(0);
    }
  g_display_name = display_name;

  if (g_display_name && strcmp (g_display_name, "drm") == 0)
    g_drm_device_name = get_drm_device_name (*argc, argv);

  sanitize_args (argc, argv);
  }
//}}}
//{{{
VADisplay va_open_display() {

  VADisplay va_dpy = NULL;

  for (unsigned int i = 0; !va_dpy && g_display_hooks_available[i]; i++) {
    g_display_hooks = g_display_hooks_available[i];
    if (g_display_name &&
        strcmp (g_display_name, g_display_hooks->name) != 0)
      continue;
    if (!g_display_hooks->open_display)
      continue;
    va_dpy = g_display_hooks->open_display();
    }

  if (!va_dpy)  {
    fprintf (stderr, "error: failed to initialize display");
    if (g_display_name)
      fprintf (stderr, " '%s'", g_display_name);
    fprintf (stderr, "\n");
    exit(1);
    }

  return va_dpy;
  }
//}}}
//{{{
void va_close_display (VADisplay va_dpy) {

  if (!va_dpy)
    return;

  if (g_display_hooks && g_display_hooks->close_display)
    g_display_hooks->close_display (va_dpy);
  }
//}}}

//{{{
VAStatus va_put_surface (VADisplay va_dpy, VASurfaceID surface,
                         const VARectangle *src_rect, const VARectangle *dst_rect) {

  if (!va_dpy)
    return VA_STATUS_ERROR_INVALID_DISPLAY;

  if (g_display_hooks && g_display_hooks->put_surface)
    return g_display_hooks->put_surface (va_dpy, surface, src_rect, dst_rect);

  return VA_STATUS_ERROR_UNIMPLEMENTED;
  }
//}}}

//{{{
void va_print_display_options (FILE* stream) {

  fprintf (stream, "Display options:\n");
  fprintf (stream, "\t--display display | help         Show information for the specified display, or the available display list \n");
  fprintf (stream, "\t--device device                  Set device name, only available under drm display\n");
  }
//}}}
