include ../../Makefile.inc

# this library doesn't require the std c library
CFLAGS+=-ffreestanding -nostdlib

STM32F4_DIR=../stm32f4
STM32F4_INCDIR=$(STM32F4_DIR)/include

CPPFLAGS+=-I$(STM32F4_INCDIR) -I$(STM32F4_INCDIR)/core -I$(STM32F4_INCDIR)/peripherals

CPPFLAGS+=-I../../include -Iinclude

#LWIP_DIR=../lwip
#LWIP_PORTDIR=$(LWIP_DIR)/port/STM32F4x7/Standalone
#LWIP_INCDIR=$(LWIP_DIR)/src/include

#CPPFLAGS+=-I$(LWIP_DIR) -I$(LWIP_INCDIR) -I$(LWIP_INCDIR)/ipv4 -I$(LWIP_PORTDIR)/include

LIB=libtm
SRCS=src/tm_stm32f4_delay.c \
     src/tm_stm32f4_dma.c \
     src/tm_stm32f4_gpio.c \
     src/tm_stm32f4_spi.c \
     src/tm_stm32f4_spi_dma.c
OBJS=$(SRCS:.c=.o)

all: $(LIB).a

$(LIB).a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIB).a
