#include "spi.h"

#include <stm32f4xx_spi.h>
#include <tm_stm32f4_gpio.h>

/**
 * SPI communication with the AD9910. Uses the TM library to make
 * programming a bit simpler. For documentation on that see here:
 * http://stm32f4-discovery.com/2014/04/library-05-spi-for-stm32f4xx/
 *
 * See the AD9910 data sheet starting on page 48 for information on the
 * serial programming interface of the DDS chip.
 */

#define SPI_DATASIZE SPI_DataSize_8b;
#define SPI_PRESCALER SPI_BaudRatePrescaler_256;

/**
 * this functions sets up all the spi communication. To only usage of SPI
 * is currently the communication between AD9910 and the STM32F4
 */
void
spi_init()
{
  SPI_InitTypeDef spi_init;

  SPI_StructInit(&spi_init);

  /* enable SPI clock */
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

  /* init pins */
  TM_GPIO_InitAlternate(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
                        TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL,
                        TM_GPIO_Speed_High, GPIO_AF_SPI1);

  /* set options */
  spi_init.SPI_DataSize = SPI_DATASIZE;
  spi_init.SPI_BaudRatePrescaler = SPI_PRESCALER;
  spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  /* SPI mode */
  spi_init.SPI_CPOL = SPI_CPOL_Low;
  spi_init.SPI_CPHA = SPI_CPHA_1Edge;

  /* disable first */
  SPI1->CR1 &= ~SPI_CR1_SPE;

  SPI_Init(SPI1, &spi_init);

  /* enable SPI */
  SPI1->CR1 |= SPI_CR1_SPE;
}

void
spi_write_multi(uint8_t* data, uint32_t length)
{
  /* TODO let that use DMA */
  SPI_WAIT(SPI1);

  for (uint32_t i = 0; i < length; ++i) {
    SPI1->DR = data[i];

    SPI_WAIT(SPI1);

    /* we need to read the data register */
    (void)SPI1->DR;
  }
}
