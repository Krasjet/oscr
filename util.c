/* util.c: common utilities */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void
die(const char *msg, ...)
{
  va_list ap;

  fputs("[error] ", stderr);

  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  putc('\n', stderr);
  va_end(ap);
  exit(1);
}

void
info(const char *msg, ...)
{
  va_list ap;

  fputs("[info] ", stdout);

  va_start(ap, msg);
  vfprintf(stdout, msg, ap);
  putc('\n', stdout);
  va_end(ap);
}

void
warn(const char *msg, ...)
{
  va_list ap;

  fputs("[warning] ", stdout);

  va_start(ap, msg);
  vfprintf(stdout, msg, ap);
  putc('\n', stdout);
  va_end(ap);
}
