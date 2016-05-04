#include "crc.h"

#include <stm32f4xx.h>
#include <stm32f4xx_crc.h>

void
crc_init()
{
  /* enable CRC clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
}

void
crc_deinit()
{
  RCC->AHB1ENR &= ~RCC_AHB1ENR_CRCEN;
}

uint32_t
crc(const uint32_t* buf, uint32_t len)
{
  CRC_ResetDR();
  return crc_continue(buf, len);
}

uint32_t
crc_continue(const uint32_t* buf, uint32_t len)
{
  /* ST apparently doesn' like const, but the function doesn' touch buf */
  return CRC_CalcBlockCRC((uint32_t*)buf, len);
}
