include ../Makefile.inc

# this library doesn't require the std c library
CFLAGS+=-ffreestanding -nostdlib

CPPFLAGS+=-I../include -Iinclude -Iinclude/core -Iinclude/peripherals

SRCS=src/stm32f4_discovery.c \
     src/system_stm32f4xx.c \
     src/peripherals/misc.c \
     src/peripherals/stm32f4x7_eth.c \
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
ASMS=src/startup_stm32f4xx.s
OBJS=$(SRCS:.c=.o) $(ASMS:.s=.o)

all: libstm32f4.a

libstm32f4.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) libstm32f4.a
