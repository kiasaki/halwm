#include "halwm.h"
#include "config.h"

xcb_connection_t *conn;
xcb_screen_t *screen;
uint16_t screen_w, screen_h;
bool run = false;
node *head = NULL;
node *current = NULL;

void die(const char *mess) {
  p(mess);
  exit(1);
}

void render_tiles() {
  uint16_t main_screen_size = screen_w * MAIN_WINDOW_SIZE;
  uint8_t pad = GAP*2 + BORDER*2;

  if (head && !head->next) {
    move_resize(head->win, GAP, GAP,
        screen_w - pad,
        screen_h - pad);
  } else {
    move_resize(head->win, GAP, GAP,
        main_screen_size - pad,
        screen_h - pad);

    node *n = head;
    uint8_t y = 0;
    uint8_t i = 0;
    while (n->next) { ++i; n = n->next; }
    uint16_t height = screen_h / i;
    for (n = head->next; i > 0; --i, n = n->next, y += height) {
      move_resize(n->win,
        main_screen_size + GAP, y + GAP,
        screen_w - main_screen_size - pad,
        ((n->next) ? height : (screen_h - y)) - pad);
    }
  }
}

void render_current() {
  node *n;
  for (n = head; n; n = n->next) {
    uint32_t b[1] = { BORDER };
    xcb_configure_window(conn, n->win, XCB_CONFIG_WINDOW_BORDER_WIDTH, b);

    if (n == current) {
      xcb_change_window_attributes(conn, n->win,
          XCB_CW_BORDER_PIXEL, &WINDOW_FOCUS);
    } else {
      xcb_change_window_attributes(conn, n->win,
          XCB_CW_BORDER_PIXEL, &WINDOW_UNFOCUS);
    }
  }
}

void node_create(xcb_window_t win) {
  node *n = (node*)calloc(1, sizeof(node));
  n->win = win;
  n->next = NULL;
  n->prev = NULL;

  if (head) {
    node *tmp = head;
    while (tmp->next) tmp = tmp->next;
    tmp->next = n;
    n->prev = tmp;
  } else {
    head = n;
  }

  current = n;
}

void node_remove(xcb_window_t win) {
  node *n = head;
  while (n) {
    if (n->win == win) {
      if (n == head) head = n->next;
      if (n->next) n->next->prev = n->prev;
      if (n->prev) n->prev->next = n->next;
      if (n == current) current = n->next ? n->next : n->prev;
      free(n);
    }
    n = n->next;
  }
}

void handle_map(xcb_map_request_event_t *ev) {
  node_create(ev->window);
  render_tiles();
  xcb_map_window(conn, ev->window);
  render_current();
}

void handle_destroy(xcb_destroy_notify_event_t *ev) {
  node_remove(ev->window);
  render_tiles();
  render_current();
}

void setup() {
  screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

  screen_w = screen->width_in_pixels;
  screen_h = screen->height_in_pixels;

  uint32_t mask[1] = {
      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | // destroy notify
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT }; // map request
  xcb_change_window_attributes(conn, screen->root, XCB_CW_EVENT_MASK, mask);
  xcb_flush(conn);

  run = true;
}

void loop() {
  xcb_generic_event_t *ev;

  do {
    if ((ev = xcb_poll_for_event(conn))) {
      switch(ev->response_type & ~0x80) {
      case XCB_MAP_REQUEST: {
        handle_map((xcb_map_request_event_t*)ev);
        xcb_flush(conn);
        } break;
      case XCB_DESTROY_NOTIFY: {
        handle_destroy((xcb_destroy_notify_event_t*)ev);
        xcb_flush(conn);
        } break;
      }
    }

    free(ev);

    struct timespec t = { 0, 30000000L };
    nanosleep(&t, NULL);
  } while (run);
}

void quit() {
  run = false;
  p("Goodbye!");

  xcb_disconnect(conn);
  exit(0);
}

int main (void) {
    conn = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(conn)) die("Can't connect to X server");

    setup();

    signal(SIGINT, quit);

    loop();

    quit();
    return 0;
}
