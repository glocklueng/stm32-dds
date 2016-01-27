/**
  ******************************************************************************
  * @file    IO_Toggle/main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include "ad9910.h"
#include "spi.h"
#include "gpio.h"

/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS 10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* this variable is used as time reference, incremented by 10ms */
__IO uint32_t LocalTime = 0;
uint32_t timingdelay;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int
main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
        system_stm32f4xx.c file
     */

  gpio_set_high(LED_ORANGE);

  gpio_init();

  spi_init();

  gpio_set_high(LED_GREEN);

  /*
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

  spi_write_single(AD9910_INSTR_WRITE | AD9910_REG_CFR1_ADDR);
  spi_write_single(AD9910_INSTR_WRITE | 0x2);
  spi_write_single(AD9910_INSTR_WRITE | 0x0);
  spi_write_single(AD9910_INSTR_WRITE | 0x0);
  spi_write_single(AD9910_INSTR_WRITE | 0x0);

  SPI_WAIT(SPI1);

  ad9910_io_update();

  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

  spi_send_single(AD9910_INSTR_READ | AD9910_REG_CFR3_ADDR);
  spi_send_single(0x55);
  spi_send_single(0x55);
  spi_send_single(0x55);
  spi_send_single(0x55);
  spi_send_single(AD9910_INSTR_READ | AD9910_REG_CFR1_ADDR);
  spi_send_single(0x55);
  spi_send_single(0x55);
  spi_send_single(0x55);
  spi_send_single(0x55);
  */

  SPI_WAIT(SPI1);
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

//  ad9910_init();

  update_reg(AD9910_REG_CFR3);
  SPI_WAIT(SPI1);
  ad9910_io_update();
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

  /* enable PLL mode */
  set_value(AD9910_PLL_ENABLE, 1);
  update_reg(AD9910_REG_CFR3);
  SPI_WAIT(SPI1);
  ad9910_io_update();
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);
  /* set multiplier factor (10MHz -> 1GHz) */
  set_value(AD9910_PLL_DIVIDE, 100);
  update_reg(AD9910_REG_CFR3);
  SPI_WAIT(SPI1);
  ad9910_io_update();
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);
  /* set correct range for internal VCO */
  set_value(AD9910_VCO_RANGE, AD9910_VCO_RANGE_VCO5);
  update_reg(AD9910_REG_CFR3);
  SPI_WAIT(SPI1);
  ad9910_io_update();
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);
  /* set pump current for the external PLL loop filter */
  set_value(AD9910_PLL_PUMP_CURRENT, AD9910_PLL_PUMP_CURRENT_237);
  update_reg(AD9910_REG_CFR3);
  SPI_WAIT(SPI1);
  ad9910_io_update();
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

  gpio_set_high(PROFILE_0);
  gpio_set_high(PROFILE_1);
  gpio_set_high(PROFILE_2);

  uint64_t f = 0x418937; // should be 10kHz at 10MHz / 1MHz at 1GHz
  uint64_t p = 0;
  uint64_t a = 0x3FFF;

  uint64_t data = f | (p << 32) | (a << 48);

  SPI_WAIT(SPI1);
  gpio_set_high(IO_RESET);
  for (volatile int i = 0; i < 1000; ++i);
  gpio_set_low(IO_RESET);

  ad9910_update_register(AD9910_GET_ADDR(AD9910_REG_PROF0), 8, &data);

  SPI_WAIT(SPI1);

  ad9910_io_update();

  gpio_set_low(PROFILE_0);
  gpio_set_low(PROFILE_1);
  gpio_set_low(PROFILE_2);

  gpio_blink_forever_slow(LED_RED);
}

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void
Delay(uint32_t nCount)
{
  /* capture the current local time */
  timingdelay = LocalTime + nCount;

  /* wait until the desired delay finishes */
  while (timingdelay > LocalTime) {
  }
}

void
Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void
assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */

  gpio_blink_forever_fast(LED_RED);
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
