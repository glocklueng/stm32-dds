#ifndef _SPI_H
#define _SPI_H

#include "util.h"

#include <stm32f4xx_spi.h>

void spi_init_slow(void);
void spi_init_fast(void);
void spi_init(uint16_t prescaler);
void spi_deinit(void);
INLINE uint8_t spi_send_single(uint8_t data);
INLINE int spi_is_busy(void);
INLINE void spi_wait(void);
void spi_write_multi(uint8_t* data, uint32_t length);

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
  return (SPI1->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 ||
         (SPI1->SR & SPI_SR_BSY);
}

INLINE void
spi_wait()
{
  while (spi_is_busy())
    ;
}

#endif /* _SPI_H */
