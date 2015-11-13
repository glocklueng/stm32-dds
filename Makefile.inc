AR=arm-none-eabi-ar
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS+=-Wall -Wextra
# processor settings for stm32f407
CFLAGS+=-mthumb -mcpu=cortex-m4

# define constant for processor type
CPPFLAGS+=-DSTM32F40_41xxx
