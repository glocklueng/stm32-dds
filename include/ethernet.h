#ifndef __ETHERNET_H
#define __ETHERNET_H

#include <lwip/err.h>
#include <stddef.h>
#include <stdint.h>

struct binary_data
{
  char name[8];
  void* begin;
  void* end;
};

void ethernet_init(void);
void ethernet_loop(void);

err_t ethernet_queue(const char*, uint16_t length);
err_t ethernet_copy_queue(const char*, uint16_t length);

/**
 * write data from the input buffer into buf
 *
 * this function is called by the lexer if it needs more data. It copies
 * data from the lwip buffer into the lexers buffer
 */
size_t ethernet_get_data(char* buf, size_t len);

/* signals the parser that it should handle binary data next */
void ethernet_data_next(struct binary_data*);

#endif /* __ETHERNET_H */
