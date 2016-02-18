#ifndef __ETHERNET_H
#define __ETHERNET_H

#include <stddef.h>

void ethernet_init(void);
void ethernet_loop(void);

void ethernet_queue(const char*, size_t length);
void ethernet_copy_queue(const char*, size_t length);

#endif /* __ETHERNET_H */
