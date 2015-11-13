include ../Makefile.inc

# this library doesn't require the std c library
CFLAGS+=-ffreestanding -nostdlib

CPPFLAGS+=-Iinclude -Iinclude/core -Iinclude/peripherals

SRCS=misc.c stm32f4xx_dma.c stm32f4xx_rcc.c stm32f4xx_adc.c \
    stm32f4xx_exti.c stm32f4xx_rng.c stm32f4xx_can.c stm32f4xx_flash.c \
    stm32f4xx_rtc.c stm32f4xx_crc.c stm32f4xx_fsmc.c stm32f4xx_sdio.c \
    stm32f4xx_cryp_aes.c stm32f4xx_gpio.c stm32f4xx_spi.c \
    stm32f4xx_cryp.c stm32f4xx_hash.c stm32f4xx_syscfg.c \
    stm32f4xx_cryp_des.c stm32f4xx_hash_md5.c stm32f4xx_tim.c \
    stm32f4xx_cryp_tdes.c stm32f4xx_hash_sha1.c  stm32f4xx_usart.c \
    stm32f4xx_dac.c stm32f4xx_i2c.c stm32f4xx_wwdg.c \
    stm32f4xx_dbgmcu.c stm32f4xx_iwdg.c \
    stm32f4xx_dcmi.c stm32f4xx_pwr.c
OBJS=$(SRCS:.c=.o)

vpath %.c src src/peripherals

all: libstm32f4.a

libstm32f4.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) libstm32f4.a
