#pragma once
#include <va/va.h>
#include <stdio.h>

//{{{
#ifdef __cplusplus
extern "C" {
#endif
//}}}

typedef struct {
  const char* name;
  VADisplay(*open_display)();
  void (*close_display)(VADisplay va_dpy);
  VAStatus(*put_surface)(VADisplay va_dpy, VASurfaceID surface,
                         const VARectangle* src_rect, const VARectangle* dst_rect);
  } VADisplayHooks;

void va_init_display_args (int* argc, char* argv[]);

VADisplay va_open_display();
void va_close_display (VADisplay va_dpy);

VAStatus va_put_surface (VADisplay va_dpy, VASurfaceID surface,
                         const VARectangle *src_rect, const VARectangle *dst_rect);

void va_print_display_options (FILE* stream);

//{{{
#ifdef __cplusplus
}
#endif
//}}}
