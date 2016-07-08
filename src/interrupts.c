#include "interrupts.h"

#include "ethernet.h"
#include "gpio.h"
#include "timing.h"

#include <misc.h>
#include <stddef.h>
#include <stm32f4xx.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_syscfg.h>

void EXTI0_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void
init_interrupts()
{
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  /* Enable clock for GPIOA */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable clock for SYSCFG */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Tell system that you will use PA0 for EXTI_Line0 */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* PA0 is connected to EXTI_Line0 */
  EXTI_InitStruct.EXTI_Line = EXTI_Line0;
  /* Enable interrupt */
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  /* Interrupt mode */
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  /* Triggers on falling edge */
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  /* Add to EXTI */
  EXTI_Init(&EXTI_InitStruct);

  /* Add IRQ vector to NVIC */
  /* PA0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
  NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
  /* Set priority */
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
  /* Set sub priority */
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
  /* Enable interrupt */
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  /* Add to NVIC */
  NVIC_Init(&NVIC_InitStruct);

  /** configure interrupts for PLL lock (Pin C15) */
  /* Enable clock for GPIOC */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  /* Tell system that you will use PC15 for EXTI_Line15 */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource15);

  EXTI_InitStruct.EXTI_Line = EXTI_Line15;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

void
EXTI0_IRQHandler()
{
  /* Make sure that interrupt flag is set */
  if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
    gpio_toggle(LED_RED);
    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

void
EXTI15_10_IRQHandler()
{
  /* this should only be called if the PLL has lost it's lock signal */
  if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
    static const char msg[] = "DDS lost PLL signal. Output will be wrong.\n";
    ethernet_queue(msg, sizeof(msg));
    EXTI_ClearITPendingBit(EXTI_Line15);
  }
}

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void
NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void
HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  const size_t cycles = 2 * 1000 * 1000;
  while (1) {
    gpio_set_high(LED_RED);
    gpio_set_high(LED_BLUE);
    gpio_set_high(LED_GREEN);
    gpio_set_high(LED_ORANGE);

    for (volatile unsigned int i = 0; i < cycles; ++i) {
    }

    gpio_set_low(LED_RED);
    gpio_set_low(LED_BLUE);
    gpio_set_low(LED_GREEN);
    gpio_set_low(LED_ORANGE);

    for (volatile unsigned int i = 0; i < cycles; ++i) {
    }
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void
MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1) {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void
BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1) {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void
UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1) {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void
SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void
DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void
PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void
SysTick_Handler(void)
{
  time_update();
}
