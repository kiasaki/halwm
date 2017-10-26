#define _POSIX_C_SOURCE 199506L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <inttypes.h>

#define XCB_MOVE XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define XCB_RESIZE XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define XCB_MOVE_RESIZE XCB_MOVE | XCB_RESIZE
#define move_resize(win, x, y, w, h) \
    uint32_t tmpvals[4] = { x, y, w, h }; \
    xcb_configure_window(conn, win, XCB_MOVE_RESIZE, tmpvals);
#define p(s) printf("halwm: %s\n", s)

typedef struct node {
    xcb_window_t win;
    struct node *next;
    struct node *prev;
} node;

const int32_t WINDOW_FOCUS = 52260;
const int32_t WINDOW_UNFOCUS = 34184;
