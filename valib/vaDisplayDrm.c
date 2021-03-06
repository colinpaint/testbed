//{{{  includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef IN_LIBVA
  # include "va/drm/va_drm.h"
#else
  # include <va/va_drm.h>
#endif

#include "va_display.h"
//}}}

static int drm_fd = -1;
extern const char* g_drm_device_name;

//{{{
static VADisplay va_open_display_drm() {

  VADisplay va_dpy;

  static const char* drm_device_paths[] = {
    "/dev/dri/renderD128",
    "/dev/dri/card0",
    NULL
    };

  if (g_drm_device_name) {
    drm_fd = open (g_drm_device_name, O_RDWR);
    if (drm_fd < 0) {
      printf ("Failed to open the given device!\n");
      exit (1);
      return NULL;
      }

    va_dpy = vaGetDisplayDRM (drm_fd);
    if (va_dpy)
      return va_dpy;

    printf ("Failed to a DRM display for the given device\n");
    close (drm_fd);
    drm_fd = -1;
    exit (1);
    return NULL;
     }

  for (int i = 0; drm_device_paths[i]; i++) {
    drm_fd = open(drm_device_paths[i], O_RDWR);
    if (drm_fd < 0)
      continue;

    va_dpy = vaGetDisplayDRM (drm_fd);
    if (va_dpy)
      return va_dpy;

    close (drm_fd);
    drm_fd = -1;
    }

  return NULL;
  }
//}}}
//{{{
static void va_close_display_drm (VADisplay va_dpy) {

  if (drm_fd < 0)
    return;

  close (drm_fd);
  drm_fd = -1;
  }
//}}}
//{{{
static VAStatus va_put_surface_drm (VADisplay  va_dpy, VASurfaceID surface,
                                    const VARectangle* src_rect, const VARectangle* dst_rect) {
  return VA_STATUS_ERROR_OPERATION_FAILED;
  }
//}}}

const VADisplayHooks va_display_hooks_drm = {
  "drm",
  va_open_display_drm,
  va_close_display_drm,
  va_put_surface_drm,
  };
