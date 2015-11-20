PROJECT_NAME=main

include Makefile.inc

SRCS=src/main.c \
     src/interrupts.c
OBJS=$(SRCS:.c=.o)

LIBS=libstm32f4.a \
     liblwip.a
LIB_FILES=$(LIBS:%=lib/%)

# set search path for include files
CPPFLAGS+=-Iinclude -Ilib/include -Ilib/include/core -Ilib/include/peripherals

## use custom linker script
LDFLAGS+=-Tstm32_flash.ld

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

clean:
	rm -f $(OBJS) $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin
	$(MAKE) -C lib clean
