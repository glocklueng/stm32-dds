PROJECT_NAME=main

include Makefile.inc

SRCS=src/main.c \
     src/ad9910.c \
     src/commands.c \
     src/dma.c \
     src/gpio.c \
     src/interrupts.c \
     src/ethernet.c \
     src/spi.c \
     src/timing.c
HDRS=include/defines.h \
     include/ad9910.h \
     include/commands.h \
     include/dma.h \
     include/ethernet.h \
     include/gpio.h \
     include/interrupts.h \
     include/main.h \
     include/spi.h \
     include/stm32f4x7_eth_conf.h \
     include/timing.h
OBJS=$(SRCS:.c=.o)

LIBS=libtm.a \
     libstm32f4.a \
     liblwip.a
LIB_FILES=$(LIBS:%=lib/%)

STM32_DIR=lib/stm32f4
LWIP_DIR=lib/lwip
TM_DIR=lib/tm

#CFLAGS+=-Wmissing-declarations -Werror=implicit-function-declaration
CFLAGS+=-Wno-unused-parameter -Winline

# set search path for include files
CPPFLAGS+=-Iinclude
CPPFLAGS+=-I$(STM32_DIR)/include -I$(STM32_DIR)/include/core -I$(STM32_DIR)/include/peripherals
CPPFLAGS+=-I$(LWIP_DIR) -I$(LWIP_DIR)/src/include -I$(LWIP_DIR)/src/include/ipv4
CPPFLAGS+=-I$(LWIP_DIR)/port/STM32F4x7/Standalone/include
CPPFLAGS+=-I$(TM_DIR)/include

.PHONY: all lib proj clean flash stlink gdb

all: proj

lib/%.a:
	$(MAKE) -C lib $(@:lib/%=%)

proj: flash.elf ram.elf

flash: flash.elf
	$(GDB) --batch --eval-command="target extended-remote :4242" \
	    --eval-command="load" --eval-command="continue" $<

debug: ram.elf
	$(GDB) --eval-command="target extended-remote :4242" \
	    --eval-command="load" $<

gdb: proj
	$(GDB) --eval-command="target extended-remote :4242" $<

stlink:
	st-util -p 4242 -s 2 -m

%.elf: src/stm32_%.ld $(OBJS) $(LIB_FILES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -T$^ -o $@ $(LDFLAGS)

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

format:
	clang-format -i $(SRCS) $(HDRS)

clean:
	rm -f $(OBJS) ram.elf flash.elf
	$(MAKE) -C lib clean
