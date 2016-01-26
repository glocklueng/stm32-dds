#ifndef _SPI_H
#define _SPI_H

#include <stm32f4xx_spi.h>

/* our used SPI channel */
#define SPIx SPI1

/* helper macros taken from tm library */
#define SPI_IS_BUSY(SPIy)                                                      \
  (((SPIy)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 || ((SPIy)->SR & SPI_SR_BSY))

#define SPI_WAIT(SPIy) while (SPI_IS_BUSY(SPIy))

void spi_init();

inline uint8_t
spi_send_single(uint8_t data)
{
  /* wait until previous transmission is complete */
  SPI_WAIT(SPIx);

  /* fill output buffer */
  SPIx->DR = data;

  SPI_WAIT(SPIx);

  /* return received data */
  return SPIx->DR;
}

inline void
spi_write_single(uint8_t data)
{
  SPI_WAIT(SPIx);

  SPIx->DR = data;
}

void spi_write_multi(uint8_t* data, uint32_t length);

#endif /* _SPI_H */
