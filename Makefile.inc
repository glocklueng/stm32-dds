ifeq ($(OS),Windows_NT)
	TOOLCHAIN_PATH?=/cygdrive/c/Program\ Files\ \(x86\)/GNU\ Tools\ ARM\ Embedded/4.9\ 2015q3/bin/
endif

AR=$(TOOLCHAIN_PATH)arm-none-eabi-ar
AS=$(TOOLCHAIN_PATH)arm-none-eabi-as
CC=$(TOOLCHAIN_PATH)arm-none-eabi-gcc
GDB=$(TOOLCHAIN_PATH)arm-none-eabi-gdb
OBJCOPY=$(TOOLCHAIN_PATH)arm-none-eabi-objcopy

CFLAGS+=-std=c99
CFLAGS+=-Wall -Wextra -ggdb -g3 -O0
# processor settings for stm32f407
CFLAGS+=-mthumb -mcpu=cortex-m4

CFLAGS+=-Werror=missing-prototypes -Werror=implicit-function-declaration

# define constant for processor type
CPPFLAGS+=-DSTM32F40_41xxx
