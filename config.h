/* config.h: config file loader */
#ifndef OSCR_CONFIG_H
#define OSCR_CONFIG_H

#include <lo/lo.h>

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
 * load config from stdin
 *
 * returns:
 *   number of loaded routes
 */
int config_load(struct oscr_config *config);

/**
 * free config struct
 *
 * ASSUMES:
 *   config != NULL
 */
void config_free(struct oscr_config *config);

#endif
