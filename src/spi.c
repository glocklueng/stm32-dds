#include "spi.h"

/**
 * SPI communication with the AD9910. Uses the TM library to make
 * programming a bit simpler. For documentation on that see here:
 * http://stm32f4-discovery.com/2014/04/library-05-spi-for-stm32f4xx/
 *
 * See the AD9910 data sheet starting on page 48 for information on the
 * serial programming interface of the DDS chip.
 */

/**
 * this functions sets up all the spi communication. To only usage of SPI
 * is currently the communication between AD9910 and the STM32F4
 */
void
spi_init()
{
  TM_SPI_Init(SPI1, TM_SPI_PinsPack_2);
}
