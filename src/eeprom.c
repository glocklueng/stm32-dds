#include "eeprom.h"

int
eeprom_write(enum eeprom_id id, uint16_t addr, void* data, size_t len)
{
  FLASH_Unlock();
  for (size_t i = 0; i < len; i++) {
    FLASH_WaitForLastOperation();
    FLASH_Status status = FLASH_ProgramByte((uint32_t)eeprom_get(id, addr + i),
                                            ((volatile uint8_t*)data)[i]);

    if (status != FLASH_COMPLETE) {
      FLASH_Lock();
      return 1;
    }
  }
  FLASH_Lock();

  return 0;
}

int
eeprom_erase(enum eeprom_id id)
{
  FLASH_Unlock();
  FLASH_Status status = FLASH_EraseSector(id, VoltageRange_3);
  FLASH_Lock();
  if (status != FLASH_COMPLETE) {
    return 1;
  } else {
    return 0;
  }
}
