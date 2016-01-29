#ifndef _SPI_H
#define _SPI_H

#include <stm32f4xx_spi.h>

#define INLINE __attribute__((always_inline)) inline

/* helper macros taken from tm library */
#define SPI_IS_BUSY(SPIx)                                                      \
  (((SPIx)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 || ((SPIx)->SR & SPI_SR_BSY))

#define SPI_WAIT(SPIx) while (SPI_IS_BUSY(SPIx))

void spi_init_slow();
void spi_init_fast();
void spi_init(uint16_t prescaler);
void spi_deinit();
INLINE uint8_t spi_send_single(uint8_t data);
void spi_write_multi(uint8_t* data, uint32_t length);

INLINE uint8_t
spi_send_single(uint8_t data)
{
  /* wait until previous transmission is complete */
  SPI_WAIT(SPI1);

  /* fill output buffer */
  SPI1->DR = data;

  SPI_WAIT(SPI1);

  /* return received data */
  return SPI1->DR;
}

#endif /* _SPI_H */
