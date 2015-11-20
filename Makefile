PROJECT_NAME=main

include Makefile.inc

SRCS=src/main.c \
     src/stm32f4xx_it.c
OBJS=$(SRCS:.c=.o)

# set search path for include files
CPPFLAGS+=-Iinclude -Ilib/include -Ilib/include/core -Ilib/include/peripherals

LDFLAGS+=-lstm32f4
## use custom linker script
LDFLAGS+=-Tstm32_flash.ld
## use library files
LDFLAGS+=-Llib

.PHONY: all lib proj clean flash stlink gdb

all: proj

lib/libstm32f4.a:
	$(MAKE) -C lib

proj: $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin

flash: $(PROJECT_NAME).elf
	$(GDB) --batch --eval-command="target extended-remote :4242" \
	    --eval-command="load" --eval-command="continue" $<

gdb: $(PROJECT_NAME).elf
	$(GDB) --eval-command="target extended-remote :4242" $<

stlink:
	st-util -p 4242 -s 2

%.elf: $(OBJS) lib/libstm32f4.a
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

%.hex: %.elf
	$(OBJCOPY) -O ihex $^ $@

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

clean:
	rm -f $(OBJS) $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin
	$(MAKE) -C lib clean
