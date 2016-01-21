/**
 * SPI communication with the AD9910. Uses the TM library to make
 * programming a bit simpler. For documentation on that see here:
 * http://stm32f4-discovery.com/2014/04/library-05-spi-for-stm32f4xx/
 */

#include "tm_stm32f4_spi.h"

/**
 * this functions sets up all the spi communication. To only usage of SPI
 * is currently the communication between AD9910 and the STM32F4
 */
void
spi_init() {
  TM_SPI_Init(SPI1, TM_SPI_PinsPack_2);
}
