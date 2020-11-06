/* oscr.c: main module for oscr */
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <lo/lo.h>

#include "config.h"
#include "util.h"

static int running;

static void
err_handler(int num, const char *msg, const char *path)
{
  (void) num;
  (void) msg;
  (void) path;
  die("fail to start osc server");
}

static int
dispatcher(const char *path, const char *types, lo_arg **argv,
           int argc, lo_message m, void* user_data)
{
  (void) types;
  (void) argv;
  (void) argc;
  struct oscr_route *route = (struct oscr_route *)user_data;

  int r = lo_send_message(route->addr, path, m);
  if (r < 0)
    warn("can't send message from %s to %s", route->from, route->to);

  return 1; /* allow same route to be handled by multiple client */
}


static void
sig_handler(int signum)
{
  (void) signum;
  putchar('\n'); /* prompt start on new line */
  running = 0;
}

static void
usage(const char *name)
{
  fprintf(stdout, "usage: %s [-uth] [-p port] config\n", name);
  fprintf(stdout, "       %s [-uth] [-p port] < config\n", name);
}

int
main(int argc, char *argv[])
{
  char *port = NULL;
  struct oscr_config config;
  FILE *config_fp = stdin;
  int ret;
  int proto = LO_DEFAULT;

  int c;
  while ((c = getopt(argc, argv, "utp:h")) != -1) {
    switch (c) {
    case 'p': /* set port */
      port = optarg;
      break;
    case 'u': /* use UDP */
      proto = LO_UDP;
      break;
    case 't': /* use TCP */
      proto = LO_TCP;
      break;
    case 'h': /* help */
      usage(argv[0]);
      return 0;
    default:
      usage(argv[0]);
      return 1;
    }
  }

  if (optind < argc) { /* read config from file */
    config_fp = fopen(argv[optind], "r");
    if (!config_fp)
      die("can't open config file");
  } else if (isatty(fileno(stdin))) { /* no input and no config in argv */
    die("no config given");
  }

  ret = config_load(&config, config_fp);
  if (ret <= 0)
    die("no routes loaded");

  if (config_fp != stdin)
    fclose(config_fp);

  lo_server_thread st = lo_server_thread_new_with_proto(port, proto, err_handler);
  switch (proto) {
  case LO_DEFAULT:
  case LO_UDP:
    info("udp server running on port %s", port);
    break;
  case LO_TCP:
    info("tcp server running on port %s", port);
    break;
  }

  /* register callback */
  struct oscr_route *r = config.routes;
  while (r) {
    lo_server_thread_add_method(st, r->from, NULL, dispatcher, r);
    info("routing %s to localhost:%s", r->from, r->to);
    r = r->next;
  }

  /* catch signal for grace shutdown */
  signal(SIGQUIT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGHUP, sig_handler);
  signal(SIGINT, sig_handler);

  lo_server_thread_start(st);

  running = 1;
  while (running) {
    sleep(1);
  }

  lo_server_thread_free(st);
  config_free(&config);

  return 0;
}
