#ifndef _CRC
#define _CRC

#include <stdint.h>

void crc_init(void);
void crc_deinit(void);

uint32_t crc(const uint32_t* buf, uint32_t len);
uint32_t crc_continue(const uint32_t* buf, uint32_t len);

#endif /* _CRC */
