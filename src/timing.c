#include "timing.h"

#include <misc.h>
#include <stdint.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>

/* this variable is used as time reference, incremented by
 * SYSTEMTICK_PERIOD_MS */
volatile uint32_t LocalTime = 0;

void
sysclock_init()
{
  /* enable systick interrupts */
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  RCC_ClocksTypeDef RCC_Clocks;

  /***************************************************************************
    NOTE:
         When using Systick to manage the delay in Ethernet driver, the Systick
         must be configured before Ethernet initialization and, the interrupt
         priority should be the highest one.
  *****************************************************************************/

  /* Configure Systick clock source as HCLK */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  /* SystTick configuration: an interrupt every 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(CORE_CLOCK_SPEED / 1000 * SYSTEMTICK_PERIOD_MS);

  /* Set Systick interrupt priority to 0*/
  NVIC_SetPriority(SysTick_IRQn, 0);
}

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
