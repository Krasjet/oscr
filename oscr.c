/* oscr.c: main module for oscr */
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <lo/lo.h>

#include "config.h"
#include "util.h"

static volatile int running;

static void
err_handler(int num, const char *msg, const char *path)
{
  switch (num) {
  case LO_NOPORT:
  case LO_UNKNOWNPROTO:
    die("fail to start osc server");
    break;
  default:
    if (path)
      warn("%s: %s", path, msg);
    else
      warn("%s", msg);
  }
}

static int
parse_proto(char c)
{
  switch (c) {
  case 'u': /* UDP */
    return LO_UDP;
  case 't': /* TCP */
    return LO_TCP;
  case 'U': /* UNIX */
    return LO_UNIX;
  default:
    return -1;
  }
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
  /*                      ^ socket will be reused here */
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
  fprintf(stdout, "usage: %s [-uth] [-U socket] [-p port] [-o proto] config\n", name);
  fprintf(stdout, "       %s [-uth] [-U socket] [-p port] [-o proto] < config\n", name);
}

int
main(int argc, char *argv[])
{
  char *port = NULL, *addr = NULL;
  struct oscr_config config;
  FILE *config_fp = stdin;
  int ret;
  int proto_recv = LO_UDP, proto_send = -1;

  int c;
  while ((c = getopt(argc, argv, "utU:op:h")) != -1) {
    switch (c) {
    case 'p': /* set port */
      port = optarg;
      break;
    case 'U': /* use UNIX */
      addr = optarg;
      /* fallthrough */
    case 'u': /* use UDP */
    case 't': /* use TCP */
      proto_recv = parse_proto(c);
      break;
    case 'o': /* outgoing protocol */
      proto_send = parse_proto(*optarg);
      if (proto_send < 0)
        die("invalid outgoing protocol");
      break;
    case 'h': /* help */
      usage(argv[0]);
      return 0;
    default:
      usage(argv[0]);
      return 1;
    }
  }

  /* if send protocol not set, prefer udp and unix */
  if (proto_send == -1) {
    if (proto_recv == LO_UNIX)
      proto_send = LO_UNIX;
    else
      proto_send = LO_UDP;
  }
  /* liblo use port to specify the addr of unix domain socket */
  if (proto_recv == LO_UNIX)
    port = addr;

  if (optind < argc) { /* read config from file */
    config_fp = fopen(argv[optind], "r");
    if (!config_fp)
      die("can't open config file");
  } else if (isatty(fileno(stdin))) { /* no input and no config in argv */
    die("no config given");
  }

  ret = config_load(&config, config_fp, proto_send);
  if (ret <= 0)
    die("no routes loaded");

  if (config_fp != stdin)
    fclose(config_fp);

  lo_server_thread st = lo_server_thread_new_with_proto(port, proto_recv, err_handler);

  switch (proto_recv) {
  case LO_UDP:
    info("udp server running on port %d", lo_server_thread_get_port(st));
    break;
  case LO_TCP:
    info("tcp server running on port %d", lo_server_thread_get_port(st));
    break;
  case LO_UNIX:
    info("unix server running on socket %d", addr);
    break;
  }

  /* register callback */
  struct oscr_route *r = config.routes;
  while (r) {
    lo_server_thread_add_method(st, r->from, NULL, dispatcher, r);
    if (proto_send == LO_UNIX)
      info("routing %s to %s", r->from, r->to);
    else
      info("routing %s to localhost:%s", r->from, r->to);
    r = r->next;
  }

  /* catch signal for grace shutdown */
  signal(SIGTERM, sig_handler);
  signal(SIGINT, sig_handler);
#ifndef _WIN32
  signal(SIGQUIT, sig_handler);
  signal(SIGHUP, sig_handler);
#endif

  lo_server_thread_start(st);

  running = 1;
  while (running) {
    sleep(1);
  }

  lo_server_thread_free(st);
  config_free(&config);

  return 0;
}
