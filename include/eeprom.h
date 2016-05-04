#ifndef _EEPROM_H
#define _EEPROM_H

#include "util.h"

#include <stddef.h>
#include <stm32f4xx.h>
#include <stm32f4xx_flash.h>

enum eeprom_id
{
  eeprom_block0 = FLASH_Sector_1,
  eeprom_block1 = FLASH_Sector_2,
  eeprom_block2 = FLASH_Sector_3,
};

int eeprom_write(enum eeprom_id id, uint16_t addr, const void* data,
                 size_t len);
int eeprom_erase(enum eeprom_id id);
INLINE void* eeprom_get(enum eeprom_id id, uint16_t addr);
INLINE void* eeprom_get_end(enum eeprom_id id);
INLINE size_t eeprom_get_size(enum eeprom_id);

INLINE void*
eeprom_get(enum eeprom_id id, uint16_t addr)
{
  return (char*)0x08000000 + id * 0x800 + addr;
}

INLINE void*
eeprom_get_end(enum eeprom_id id)
{
  return (char*)0x08000000 + id * 0x800 + 0x4000;
}

INLINE size_t
eeprom_get_size(enum eeprom_id id)
{
  (void)id;
  return 0x4000;
}

#endif // _EEPROM_H
