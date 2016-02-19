#ifndef __ETHERNET_H
#define __ETHERNET_H

#include <lwip/err.h>
#include <stddef.h>
#include <stdint.h>

struct server_state;

void ethernet_init(void);
void ethernet_loop(void);

err_t ethernet_queue(struct server_state*, const char*, uint16_t length);
err_t ethernet_copy_queue(struct server_state*, const char*, uint16_t length);

#endif /* __ETHERNET_H */
