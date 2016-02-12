#ifndef _DMA_H
#define _DMA_H

#include <stm32f4xx.h>

uint32_t dma_get_stream_number(int dma, DMA_Stream_TypeDef* stream);
void dma_clear_flag(DMA_Stream_TypeDef* stream, uint32_t flag);
void dma_enable_interrupts(DMA_Stream_TypeDef* stream);
void dma_disable_interrupts(DMA_Stream_TypeDef* stream);

/* dma flags */
static const uint32_t dma_flag_tcif = 0x20;  /* transfer complete */
static const uint32_t dma_flag_htif = 0x10;  /* half transfer complete */
static const uint32_t dma_flag_teif = 0x08;  /* transfer error */
static const uint32_t dma_flag_dmeif = 0x04; /* direct mode error */
static const uint32_t dma_flag_feif = 0x01;  /* FIFO error */
static const uint32_t dma_flag_all = 0x3D;

#endif /* _DMA_H */
