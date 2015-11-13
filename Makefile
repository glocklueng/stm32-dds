PROJECT_NAME=main

include Makefile.inc

SRCS=src/main.c \
     src/stm32f4xx_it.c \
     src/system_stm32f4xx.c
OBJS=$(SRCS:.c=.o)

# set search path for include files
CPPFLAGS+=-Iinclude -Ilib/include -Ilib/include/core -Ilib/include/peripherals

LDFLAGS+=-lstm32f4
## use custom linker script
LDFLAGS+=-Tstm32_flash.ld
## use library files
LDFLAGS+=-Llib

.PHONY: all lib proj clean

all: proj

lib/libstm32f4.a:
	$(MAKE) -C lib

proj: $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin

%.elf: $(OBJS) lib/libstm32f4.a
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

clean:
	rm -f $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin
	$(MAKE) -C lib clean
