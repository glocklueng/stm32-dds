#ifndef _TIMING_H
#define _TIMING_H

#include <stdint.h>

#define CORE_CLOCK_SPEED 168000000
#define SYSTEMTICK_PERIOD_MS 1

extern volatile uint32_t LocalTime;

void sysclock_init(void);

void delay(uint32_t nCount);
void time_update(void);

#endif /* _TIMING_H */
