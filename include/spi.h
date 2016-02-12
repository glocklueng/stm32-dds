#ifndef _SPI_H
#define _SPI_H

#include "defines.h"

#include <stm32f4xx_dma.h>
#include <stm32f4xx_spi.h>

#define SPI1_DMA_TX_STREAM DMA2_Stream3
#define SPI1_DMA_TX_CHANNEL DMA_Channel_3
#define SPI1_DMA_RX_STREAM DMA2_Stream2
#define SPI1_DMA_RX_CHANNEL DMA_Channel_3

void spi_init_slow(void);
void spi_init_fast(void);
void spi_init(uint16_t prescaler);
void spi_init_dma(void);
void spi_deinit(void);
INLINE uint8_t spi_send_single(uint8_t data);
INLINE int spi_is_busy(void);
INLINE void spi_wait(void);
void spi_send_multi(const uint8_t* data, uint16_t length);

/* implementation starts here */

INLINE uint8_t
spi_send_single(uint8_t data)
{
  /* wait until previous transmission is complete */
  spi_wait();

  /* fill output buffer */
  SPI1->DR = data;

  spi_wait();

  /* return received data */
  return SPI1->DR;
}

INLINE int
spi_is_busy()
{
  return SPI1_DMA_TX_STREAM->NDTR || SPI1_DMA_RX_STREAM->NDTR ||
         (SPI1->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 ||
         (SPI1->SR & SPI_SR_BSY);
}

INLINE void
spi_wait()
{
  while (spi_is_busy())
    ;
}

#endif /* _SPI_H */
