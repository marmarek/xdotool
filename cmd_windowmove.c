#include "xdo_cmd.h"

int cmd_windowmove(context_t *context) {
  int ret = 0;
  Window wid;
  char *cmd = *context->argv;
  int opsync = 0;
  int x, y;

  int c;
  typedef enum {
    opt_unused, opt_help, opt_sync
  } optlist_t;
  static struct option longopts[] = {
    { "help", no_argument, NULL, opt_help },
    { "sync", no_argument, NULL, opt_sync },
    { 0, 0, 0, 0 },
  };
  static const char *usage = 
    "Usage: %s [options] wid x y\n"
    "--sync    - only exit once the window has moved\n";

  int option_index;
  while ((c = getopt_long_only(context->argc, context->argv, "h",
                               longopts, &option_index)) != -1) {
    switch (c) {
      case 'h':
      case opt_help:
        printf(usage, cmd);
        consume_args(context, context->argc);
        return EXIT_SUCCESS;
        break;
      case opt_sync:
        opsync = 1;
        break;
      default:
        fprintf(stderr, usage, cmd);
        return EXIT_FAILURE;
    }
  }

  consume_args(context, optind);

  if (context->argc < 3) {
    fprintf(stderr, usage, cmd);
    return 1;
  }

  wid = (Window)strtol(context->argv[0], NULL, 0);
  x = (int)strtol(context->argv[1], NULL, 0);
  y = (int)strtol(context->argv[2], NULL, 0);

  consume_args(context, 3);

  int orig_win_x = 0;
  int orig_win_y = 0;
  if (opsync) {
    xdo_get_window_location(context->xdo, wid, &orig_win_x, &orig_win_y, NULL);
  }

  ret = xdo_window_move(context->xdo, wid, x, y);
  if (ret) {
    fprintf(stderr, "xdo_window_move reported an error\n");
  } else {
    if (opsync) {
      /* This 'sync' request is stateful (we need to know the original window
       * location to make the decision about 'done' 
       * Some window managers force alignments or otherwise mangle move
       * requests, so we can't just look for the x,y positions exactly.
       * Just look for any change in the window's position. */
      int win_x, win_y;
      xdo_get_window_location(context->xdo, wid, &win_x, &win_y, NULL);
      while (orig_win_x == win_x && orig_win_y == win_y
             && win_x != x && win_y != y) {
        xdo_get_window_location(context->xdo, wid, &win_x, &win_y, NULL);
        usleep(30000);
      }
    }
  }

  return ret;
}

