include ../Makefile.inc

# this library doesn't require the std c library
CFLAGS+=-ffreestanding -nostdlib

CPPFLAGS+=-Iinclude -Iinclude/core -Iinclude/peripherals

STM32_SRCS=src/stm32f4_discovery.c \
     src/peripherals/misc.c \
     src/peripherals/stm32f4xx_adc.c \
     src/peripherals/stm32f4xx_can.c \
     src/peripherals/stm32f4xx_cec.c \
     src/peripherals/stm32f4xx_crc.c \
     src/peripherals/stm32f4xx_cryp_aes.c \
     src/peripherals/stm32f4xx_cryp.c \
     src/peripherals/stm32f4xx_cryp_des.c \
     src/peripherals/stm32f4xx_cryp_tdes.c \
     src/peripherals/stm32f4xx_dac.c \
     src/peripherals/stm32f4xx_dbgmcu.c \
     src/peripherals/stm32f4xx_dcmi.c \
     src/peripherals/stm32f4xx_dma2d.c \
     src/peripherals/stm32f4xx_dma.c \
     src/peripherals/stm32f4xx_dsi.c \
     src/peripherals/stm32f4xx_exti.c \
     src/peripherals/stm32f4xx_flash.c \
     src/peripherals/stm32f4xx_flash_ramfunc.c \
     src/peripherals/stm32f4xx_fmpi2c.c \
     src/peripherals/stm32f4xx_fsmc.c \
     src/peripherals/stm32f4xx_gpio.c \
     src/peripherals/stm32f4xx_hash.c \
     src/peripherals/stm32f4xx_hash_md5.c \
     src/peripherals/stm32f4xx_hash_sha1.c \
     src/peripherals/stm32f4xx_i2c.c \
     src/peripherals/stm32f4xx_iwdg.c \
     src/peripherals/stm32f4xx_lptim.c \
     src/peripherals/stm32f4xx_ltdc.c \
     src/peripherals/stm32f4xx_pwr.c \
     src/peripherals/stm32f4xx_qspi.c \
     src/peripherals/stm32f4xx_rcc.c \
     src/peripherals/stm32f4xx_rng.c \
     src/peripherals/stm32f4xx_rtc.c \
     src/peripherals/stm32f4xx_sai.c \
     src/peripherals/stm32f4xx_sdio.c \
     src/peripherals/stm32f4xx_spdifrx.c \
     src/peripherals/stm32f4xx_spi.c \
     src/peripherals/stm32f4xx_syscfg.c \
     src/peripherals/stm32f4xx_tim.c \
     src/peripherals/stm32f4xx_usart.c \
     src/peripherals/stm32f4xx_wwdg.c
STM32_ASMS=src/startup_stm32f4xx.s
STM32_OBJS=$(STM32_SRCS:.c=.o) $(STM32_ASMS:.s=.o)

TM_SRCS=src/tm_stm32f4_gpio.c \
     src/tm_stm32f4_exti.c
TM_OBJS=$(TM_SRCS:.c=.o)

OBJS=$(STM32_OBJS) $(TM_OBJS)

ARS=libstm32f4.a libtm.a

all: $(ARS)

libstm32f4.a: $(STM32_OBJS)
	$(AR) -r $@ $^

libtm.a: $(TM_OBJS)
	$(AR) -r $@ $^

clean:
	rm -f $(OBJS) $(ARS)
