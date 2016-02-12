#include "dma.h"

#include <misc.h>

const static IRQn_Type DMA_IRQs[2][8] = {
  { DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn,
    DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn,
    DMA1_Stream7_IRQn },
  { DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn,
    DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn }
};

uint32_t
dma_get_stream_number(int dma, DMA_Stream_TypeDef* stream)
{
  if (dma == 1) {
    return ((uint32_t)stream - (uint32_t)DMA1_Stream0) / 0x18;
  } else { // dma == 2
    return ((uint32_t)stream - (uint32_t)DMA2_Stream0) / 0x18;
  }
}

void
dma_clear_flag(DMA_Stream_TypeDef* stream, uint32_t flag)
{
  uint32_t stream_number;
  volatile uint32_t* ptr;
  if (stream < DMA2_Stream0) {
    stream_number = dma_get_stream_number(1, stream);
    ptr = (volatile uint32_t*)DMA1->LIFCR;
  } else {
    stream_number = dma_get_stream_number(2, stream);
    ptr = (volatile uint32_t*)DMA2->LIFCR;
  }

  if (stream_number >= 4) {
    ptr++;
    stream_number -= 4;
  }

  static const uint8_t dma_bit_pos[4] = { 0, 6, 16, 22 };

  *ptr = (flag & dma_flag_all) << dma_bit_pos[stream_number];
}

void
dma_enable_interrupts(DMA_Stream_TypeDef* stream)
{
  NVIC_InitTypeDef init_struct;
  uint32_t stream_number;

  dma_clear_flag(stream, dma_flag_all);

  if (stream < DMA2_Stream0) {
    stream_number = dma_get_stream_number(1, stream);
    init_struct.NVIC_IRQChannelPreemptionPriority = 1;
    init_struct.NVIC_IRQChannel = DMA_IRQs[0][stream_number];
  } else {
    stream_number = dma_get_stream_number(2, stream);
    init_struct.NVIC_IRQChannelPreemptionPriority = 1;
    init_struct.NVIC_IRQChannel = DMA_IRQs[1][stream_number];
  }

  /* Fill NVIC */
  init_struct.NVIC_IRQChannelCmd = ENABLE;
  init_struct.NVIC_IRQChannelSubPriority = stream_number;

  /* Init NVIC */
  NVIC_Init(&init_struct);

  /* enable DMA stream interrupts */
  stream->CR |= DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;
  stream->FCR |= DMA_SxFCR_FEIE;
}

void
dma_disable_interrupts(DMA_Stream_TypeDef* stream)
{
  dma_clear_flag(stream, dma_flag_all);

  const int i = !!(stream < DMA2_Stream0);

  /* Disable NVIC */
  NVIC_DisableIRQ(DMA_IRQs[i][dma_get_stream_number(i + 1, stream)]);

  /* Disable DMA stream interrupts */
  stream->CR &=
    ~(DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
  stream->FCR &= DMA_SxFCR_FEIE;
}
