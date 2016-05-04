#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdint.h>

struct ethernet_config
{
  uint8_t address[4];
  uint8_t submask[4];
  uint8_t gateway[4];
};

struct config
{
  struct ethernet_config ethernet;
};

const struct config* config_get(void);
void config_write(const struct config*);

#endif /* _CONFIG_H */
