#include "commands.h"

#include "ad9910.h"
#include "gpio.h"

/* these registers are used to keep track of the necessary changes while
 * programming the DDS. This is necessary because some changes depend on
 * the settings in other modes. Examples are the usage of the ftw register
 * instead of the profile register in some cases */
ad9910_registers command_regs;

void commands_queue_register(const ad9910_command_register* cmd)
{
}
