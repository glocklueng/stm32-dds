#ifndef _SPI_H
#define _SPI_H

#include <tm_stm32f4_spi.h>

void spi_init();

inline void
spi_write_single(uint8_t data)
{
  TM_SPI_Send(SPI1, data);
}

inline void
spi_write_multi(uint8_t* data, uint32_t length)
{
  TM_SPI_WriteMulti(SPI1, data, length);
}
#endif /* _SPI_H */
