PROJECT_NAME=main

include Makefile.inc

SRCS=src/main.c \
     src/interrupts.c \
     src/ethernet.c \
     src/netconf.c \
     src/stm32f4x7_eth_bsp.c
HDRS=include/defines.h \
     include/ethernet.h \
     include/interrupts.h \
     include/main.h \
     include/netconf.h \
     include/stm32f4x7_eth_bsp.h \
     include/stm32f4x7_eth_conf.h
OBJS=$(SRCS:.c=.o)

LIBS=libstm32f4.a \
     liblwip.a
LIB_FILES=$(LIBS:%=lib/%)

STM32_DIR=lib/stm32f4
LWIP_DIR=lib/lwip

# set search path for include files
CPPFLAGS+=-Iinclude
CPPFLAGS+=-I$(STM32_DIR)/include -I$(STM32_DIR)/include/core -I$(STM32_DIR)/include/peripherals
CPPFLAGS+=-I$(LWIP_DIR) -I$(LWIP_DIR)/src/include -I$(LWIP_DIR)/src/include/ipv4
CPPFLAGS+=-I$(LWIP_DIR)/port/STM32F4x7/Standalone/include

## use custom linker script
LDFLAGS+=-Tsrc/stm32_flash.ld

.PHONY: all lib proj clean flash stlink gdb

all: proj

lib/%.a:
	$(MAKE) -C lib $(@:lib/%=%)

proj: $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin

flash: $(PROJECT_NAME).elf
	$(GDB) --batch --eval-command="target extended-remote :4242" \
	    --eval-command="load" --eval-command="continue" $<

gdb: $(PROJECT_NAME).elf
	$(GDB) --eval-command="target extended-remote :4242" $<

stlink:
	st-util -p 4242 -s 2

%.elf: $(OBJS) $(LIB_FILES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

format:
	clang-format -i $(SRCS) $(HDRS)

clean:
	rm -f $(OBJS) $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin
	$(MAKE) -C lib clean
