# About
This is a program for the STM32F4 Discovery board with a custom shield
carrying a AD9910 DDS chip. The device is designed to run in cycles
between to different modes. During the setup mode it is receiving
information via Ethernet from a remote control program. Once all data is
received the program switches into run mode. In this mode all
communication is disabled to prevent interrupts. In this mode the program
waits for an outside trigger on which it changes the settings of the DDS
chip based on the previously send information. Once all output has been
written the program switches back into the setup mode.

# Programming modes
The settings of the DDS chip can be changed via two different ways. Via
serial communication different registers of the DDS can be changed which
become active on a trigger pulse. Alternatively parallel communication can
be used to change 16 bit of a single register with an update frequency of
up to ~22 MHz (processor speed / 6).

## Serial communication
Via serial communication registers for frequency output or of the ramp
generator can be changed while the program is running. The chip will write
the data to the DDS immediately, but they remain inactive until a trigger
signal is received. This can be repeated multiple times, e.g. to generate
a complex frequency ramp by reprogramming the ramp generator.

The changes become active immediately after triggering, however there is a
minimal delay between two triggers which is required for the processor to
write the next set of values.

## Parallel communication
Parallel communication allows to update a single register while the
DDS chip is running. The limiting update frequency is the processor speed
combined with the limited amount of RAM to store the data. The program
waits for an external trigger on which it starts writing the programmed
data to the external output. There is a time delay between sending the
trigger on reaction of the processor of one interrupt cycle.

# TODOs
- Measure update time in serial mode

# Timing information
- Interupt delay: ~400ns
