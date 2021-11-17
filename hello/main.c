//{{{  includes
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#include "cat.h"

#include "xdg-shell-client-protocol.h"
//}}}

static const int width = 128;
static const int height = 128;

static bool running = true;

static struct wl_shm* shm = NULL;
static struct wl_compositor* compositor = NULL;
static struct xdg_wm_base* xdg_wm_base = NULL;

static void* shm_data = NULL;
static struct wl_surface* surface = NULL;
static struct xdg_toplevel* xdg_toplevel = NULL;


//{{{
static void xdg_surface_handle_configure (void* data, struct xdg_surface* xdg_surface, uint32_t serial) {

  xdg_surface_ack_configure (xdg_surface, serial);
  wl_surface_commit (surface);
  }
//}}}
static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_handle_configure,
  };

static void noop() {}
//{{{
static void xdg_toplevel_handle_close (void* data, struct xdg_toplevel* xdg_toplevel) {
  running = false;
  }
//}}}
static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = noop,
  .close = xdg_toplevel_handle_close,
  };

//{{{
static void pointer_handle_button (void* data, struct wl_pointer* pointer,
                                   uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

  struct wl_seat* seat = data;
  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
    xdg_toplevel_move (xdg_toplevel, seat, serial);
    }
  }
//}}}
static const struct wl_pointer_listener pointer_listener = {
  .enter = noop,
  .leave = noop,
  .motion = noop,
  .button = pointer_handle_button,
  .axis = noop,
  };

//{{{
static void seat_handle_capabilities (void* data, struct wl_seat* seat, uint32_t capabilities) {

  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    struct wl_pointer* pointer = wl_seat_get_pointer (seat);
    wl_pointer_add_listener (pointer, &pointer_listener, seat);
    }
  }
//}}}
static const struct wl_seat_listener seat_listener = {
  .capabilities = seat_handle_capabilities,
  };

//{{{
static void handle_global (void* data, struct wl_registry* registry,
                           uint32_t name, const char*interface, uint32_t version) {

  if (strcmp (interface, wl_shm_interface.name) == 0)
    shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);

  else if (strcmp (interface, wl_seat_interface.name) == 0) {
    struct wl_seat* seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener (seat, &seat_listener, NULL);
    }

  else if (strcmp (interface, wl_compositor_interface.name) == 0)
    compositor = wl_registry_bind (registry, name, &wl_compositor_interface, 1);

  else if (strcmp (interface, xdg_wm_base_interface.name) == 0)
    xdg_wm_base = wl_registry_bind (registry, name, &xdg_wm_base_interface, 1);
  }
//}}}
static void handle_global_remove (void* data, struct wl_registry* registry, uint32_t name) {}
static const struct wl_registry_listener registry_listener = {
  .global = handle_global,
  .global_remove = handle_global_remove,
  };

//{{{
static void randomName (char* buf) {

  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r&15) + (r&16)*2;
    r >>= 5;
    }
  }
//}}}
//{{{
static int anonymousShmOpen() {

  char name[] = "/hello-wayland-XXXXXX";
  int retries = 100;

  do {
    randomName (name + strlen(name) - 6);

    --retries;

    // shm_open guarantees that O_CLOEXEC is set
    int fd = shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink (name);
      return fd;
      }
    } while (retries > 0 && errno == EEXIST);

  return -1;
  }
//}}}
//{{{
static int createShmFile (off_t size) {

  int fd = anonymousShmOpen();
  if (fd < 0) 
    return fd;

  if (ftruncate (fd, size) < 0) {
    close (fd);
    return -1;
    }

  return fd;
  }
//}}}
//{{{
static struct wl_buffer* createBuffer() {

  int stride = width * 4;
  int size = stride * height;

  int fd = createShmFile (size);
  if (fd < 0) {
    printf ("creating a buffer file for %d B failed", size);
    return NULL;
    }

  shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
    printf ("mmap failed");
    close (fd);
    return NULL;
    }

  struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);

  // MagickImage is from cat.h
  memcpy (shm_data, MagickImage, size);
  return buffer;
  }
//}}}

//{{{
int main (int argc, char* argv[]) {

  struct wl_display* display = wl_display_connect (NULL);
  if (display == NULL) {
    printf ("failed to create display\n");
    return EXIT_FAILURE;
    }

  struct wl_registry* registry = wl_display_get_registry (display);
  wl_registry_add_listener (registry, &registry_listener, NULL);
  wl_display_roundtrip (display);

  if (shm == NULL || compositor == NULL || xdg_wm_base == NULL) {
    printf ("no wl_shm, wl_compositor or xdg_wm_base support\n");
    return EXIT_FAILURE;
    }

  struct wl_buffer *buffer = createBuffer();
  if (buffer == NULL) {
    return EXIT_FAILURE;
    }

  surface = wl_compositor_create_surface (compositor);
  struct xdg_surface* xdg_surface = xdg_wm_base_get_xdg_surface (xdg_wm_base, surface);
  xdg_toplevel = xdg_surface_get_toplevel (xdg_surface);

  xdg_surface_add_listener (xdg_surface, &xdg_surface_listener, NULL);
  xdg_toplevel_add_listener (xdg_toplevel, &xdg_toplevel_listener, NULL);

  wl_surface_commit (surface);
  wl_display_roundtrip (display);

  wl_surface_attach (surface, buffer, 0, 0);
  wl_surface_commit (surface);

  while (wl_display_dispatch (display) != -1 && running) {
    // This space intentionally left blank
    }

  xdg_toplevel_destroy (xdg_toplevel);
  xdg_surface_destroy (xdg_surface);
  wl_surface_destroy (surface);
  wl_buffer_destroy (buffer);

  return EXIT_SUCCESS;
  }
//}}}
