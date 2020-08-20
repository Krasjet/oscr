/* config.c: config file loader */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "util.h"

enum Direction {
  DIR_LEFT,
  DIR_RIGHT
};

/* insert to the end, just to make sure routes are handled in order */
static void
route_insert(struct oscr_config *config, struct oscr_route *route)
{
  struct oscr_route *r = config->routes;

  if (!r) {
    config->routes = route;
    return;
  }

  while (r->next)
    r = r->next;

  r->next = route;
}

static void
route_free(struct oscr_route * r)
{
    free(r->from);
    free(r->to);
    lo_address_free(r->addr);
    free(r);
}

/**
 * read a lexeme from s to out
 *
 * note that *out is guarenteed to not be NULL and needs to be freed if
 * ret != NULL
 *
 * returns:
 *   pointer to the character after the word been read,
 *   or NULL if no word is read or no memory left
 *
 * examples:
 *   ret = lexme("  test abc", &out)
 *   ==> out = "test"
 *       ret = " abc"
 *
 *   ret = lexme("  ", &out)
 *   ==> ret = NULL
 */
static const char *
lexeme(const char *s, char **out) {
  const char * beg;
  /* skip leading spaces */
  while (isspace(*s))
    ++s;

  if (!*s)
    /* reached end of string */
    return NULL;

  beg = s;
  while(*s && !isspace(*s))
    ++s;

  /* duplicate string */
  *out = calloc(s - beg + 1, sizeof(char));
  if (!*out)
    return NULL;
  memcpy(*out, beg, s - beg);

  return s;
}

static const char *
parse_direction(const char *s, enum Direction *dir)
{
  while (isspace(*s))
    ++s;

  if (s[0] == '-' && s[1] == '>')
    *dir = DIR_RIGHT;
  else if (s[0] == '<' && s[1] == '-')
    *dir = DIR_LEFT;
  else
    return NULL;

  return s + 2;
}

int
config_load(struct oscr_config *config)
{
  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  size_t lineno = 1;
  int count = 0;

  config->routes = NULL;

  while ((len = getline(&line, &cap, stdin)) > 0) {
    const char *s = line;
    char *fst, *snd;
    enum Direction dir;
    struct oscr_route *route;

    /* is comment? */
    while (isspace(*s))
      ++s;
    if (*s == '#')
      goto nextline;

    /* first argument */
    s = lexeme(s, &fst);
    if (!s) {
      goto nextline;
    }

    /* assignment direction */
    s = parse_direction(s, &dir);
    if (!s) {
      warn("invalid direction, skipping line %lu", lineno);
      free(fst);
      goto nextline;
    }

    /* second argument */
    s = lexeme(s, &snd);
    if (!s) {
      warn("missing second argument, skipping line %lu", lineno);
      free(fst);
      goto nextline;
    }

    /* insert new route */
    route = calloc(1, sizeof(struct oscr_route));
    if (!route) {
      warn("unable to allocate route, skipping line %lu", lineno);
      free(fst);
      free(snd);
      goto nextline;
    }
    switch (dir) {
    case DIR_LEFT:
      route->from = snd;
      route->to = fst;
      break;
    case DIR_RIGHT:
      route->from = fst;
      route->to = snd;
      break;
    }
    route->addr = lo_address_new(NULL, route->to);
    if (!route->addr) {
      warn("unable to create address, skipping line %lu", lineno);
      route_free(route);
      goto nextline;
    }
    route_insert(config, route);
    count++;

nextline:
    lineno++;
  }

  if (line)
    free(line);

  return count;
}

void
config_free(struct oscr_config *config)
{
  struct oscr_route *r = config->routes;

  while (r) {
    struct oscr_route *next = r->next;
    route_free(r);
    r = next;
  }
}
