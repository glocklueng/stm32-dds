#include "spi.h"

#include "dma.h"

#include <stm32f4xx_dma.h>
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

static DMA_InitTypeDef tx_struct;
static DMA_InitTypeDef rx_struct;
static uint32_t rx_dummy_buffer;

static int dma_enabled = 0;

void
spi_init_slow()
{
  spi_init(SPI_BaudRatePrescaler_256);
}

void
spi_init_fast()
{
  spi_init(SPI_BaudRatePrescaler_2);
}

/**
 * this functions sets up all the spi communication. To only usage of SPI
 * is currently the communication between AD9910 and the STM32F4
 */
void
spi_init(uint16_t prescaler)
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
  spi_init.SPI_DataSize = SPI_DataSize_8b;
  spi_init.SPI_BaudRatePrescaler = prescaler;
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
spi_init_dma()
{
  /* enable DMA2 clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  /* we don't use the struct here, we just prepare all the settings */
  tx_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  tx_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  tx_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  tx_struct.DMA_Mode = DMA_Mode_Normal;
  tx_struct.DMA_Priority = DMA_Priority_Low;
  tx_struct.DMA_FIFOMode = DMA_FIFOMode_Disable;
  tx_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  tx_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  tx_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

  tx_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  tx_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

  tx_struct.DMA_Channel = SPI1_DMA_TX_CHANNEL;
  tx_struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;

  tx_struct.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
  tx_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;

  /* rx buffer */
  rx_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  rx_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  rx_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  rx_struct.DMA_Mode = DMA_Mode_Normal;
  rx_struct.DMA_Priority = DMA_Priority_Low;
  rx_struct.DMA_FIFOMode = DMA_FIFOMode_Disable;
  rx_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  rx_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  rx_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  rx_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  rx_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

  rx_struct.DMA_Channel = SPI1_DMA_RX_CHANNEL;
  rx_struct.DMA_DIR = DMA_DIR_PeripheralToMemory;

  rx_struct.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;

  /* change this if you wan't to receive something using DMA */
  rx_struct.DMA_MemoryInc = DMA_MemoryInc_Disable;
  rx_struct.DMA_Memory0BaseAddr = (uint32_t)&rx_dummy_buffer;

  /* enable DMA interrupts */
  dma_enable_interrupts(SPI1_DMA_RX_STREAM);
  dma_enable_interrupts(SPI1_DMA_TX_STREAM);

  dma_enabled = 1;
}

void
spi_deinit()
{
  spi_wait();

  DMA_DeInit(SPI1_DMA_TX_STREAM);
  DMA_DeInit(SPI1_DMA_RX_STREAM);
  dma_enabled = 0;
  SPI_I2S_DeInit(SPI1);
}

void
spi_send_multi(const uint8_t* data, uint16_t length)
{
  /* we first wait for other transmissions to finish */
  spi_wait();

  if (dma_enabled) {
    tx_struct.DMA_BufferSize = length;
    tx_struct.DMA_Memory0BaseAddr = (uint32_t)data;

    rx_struct.DMA_BufferSize = length;

    dma_clear_flag(SPI1_DMA_RX_STREAM, dma_flag_all);
    dma_clear_flag(SPI1_DMA_TX_STREAM, dma_flag_all);

    DMA_Init(SPI1_DMA_TX_STREAM, &tx_struct);
    DMA_Init(SPI1_DMA_RX_STREAM, &rx_struct);

    SPI1_DMA_RX_STREAM->CR |= DMA_SxCR_EN;
    SPI1_DMA_TX_STREAM->CR |= DMA_SxCR_EN;

    SPI1->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;
  } else { /* no dma */
    for (uint16_t i = 0; i < length; ++i) {
      spi_send_single(*data++);
    }
  }
}
