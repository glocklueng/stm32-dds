AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CC=arm-none-eabi-gcc
GDB=arm-none-eabi-gdb
OBJCOPY=arm-none-eabi-objcopy

CFLAGS+=-std=c99
CFLAGS+=-Wall -Wextra -ggdb -O2
# processor settings for stm32f407
CFLAGS+=-mthumb -mcpu=cortex-m4

# define constant for processor type
CPPFLAGS+=-DSTM32F40_41xxx
