# Dependencies
This program is set up to be compiled using make on a Unix system. If you
want to compile it under Windows you needto install cygwin. You also have
to make sure that make is installed on that system (Choose GNU make if you
have several options and don't know which one to take).

Since the STM32F4 is an ARM Cortex-M4 you need to have a cross compiler
for ARM. The easiest way to get one is from the GCC ARM Embedded compiler
from http://launchpad.net/gcc-arm-embedded which is available for Linux,
Mac and Windows. Version 4.9 2015q3 was used during development, but go
for the most recent version you can get.

Depending on were and how you installed the ARM toolchain it might be that
the binaries are not in your PATH (try to run arm-none-eabi-gcc, if it
works you're fine). If ot you need to find the folder the binaries are
located and set the correct path as TOOLCHAIN_PATH in Makefile.inc.

To transfer the generated binary to the microprocessor you can use stlink
under linux (found at https://github.com/texane/stlink). Follow the
compilation instructions on the project.  If you installed stlink
correctly you should be able to execute st-util.  If not you can adjust
STUTIL in Makefile.inc to point to the binary.

# Building
Once everything is set up a simple call of "make" should build the whole
project and generate a main.elf file.

# Transfering binary to the microprocessor
## Linux
Make sure that the board is connected to your pc and run "make stlink" to
start the communication server. This needs to be running while you are
working with the board. If you disconnect it you may need to restart the
server by killing it with Ctrl+C and restarting it.

While the server is running you can use gdb to debug the processor and to
transfer the binary. For convenience you can use "make gdb" to open a gdb
session which connects to the board. If you just want to write the binary
to the memory you can use "make flash" which does that for you.
