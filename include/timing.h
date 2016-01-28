#ifndef _TIMING_H
#define _TIMING_H

#include <stdint.h>

extern volatile uint32_t LocalTime;

void delay(uint32_t nCount);
void time_update();

#endif /* _TIMING_H */
