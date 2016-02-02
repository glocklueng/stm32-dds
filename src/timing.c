#include "timing.h"

#include <stdint.h>

#define SYSTEMTICK_PERIOD_MS 10
/* this variable is used as time reference, incremented by 10ms */
volatile uint32_t LocalTime = 0;

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void
delay(uint32_t nCount)
{
  /* capture the current local time */
  uint32_t timingdelay = LocalTime + nCount;

  /* wait until the desired delay finishes */
  while (timingdelay > LocalTime) {
  }
}

void
time_update()
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}
