/* config.h: config file loader */
#ifndef OSCR_CONFIG_H
#define OSCR_CONFIG_H

#include <lo/lo.h>
#include <stdio.h>

struct oscr_route {
  char *from;
  char *to;
  lo_address addr;
  struct oscr_route *next;
};

struct oscr_config {
  struct oscr_route * routes;
};

/**
 * load config from stream
 *
 * ASSUMES:
 *   f != NULL
 *
 * returns:
 *   number of loaded routes
 */
int config_load(struct oscr_config *config, FILE *f, int proto);

/**
 * free config struct
 *
 * ASSUMES:
 *   config != NULL
 */
void config_free(struct oscr_config *config);

#endif
