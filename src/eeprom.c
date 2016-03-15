#include "eeprom.h"

static FLASH_Status EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);

//--------------------------------------------------------------
// init vom virtuellen EEprom
// Return_wert :
//  -> ERROR   , wenn Fehler beim Init
//  -> SUCCESS , wenn Init vom virtuellen EEProm ok
//--------------------------------------------------------------
ErrorStatus
UB_EE_FLASH_Init(void)
{
  uint16_t PageStatus0, PageStatus1;
  uint16_t VarIdx;
  uint16_t status_wert;
  int16_t x = -1;
  int32_t read_wert;
  uint16_t data_wert;

  FLASH_Unlock();

  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  switch (PageStatus0) {
    case EE_ERASED:
      if (PageStatus1 == EE_VALID_PAGE) {
        status_wert = FLASH_EraseSector(PAGE0_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else if (PageStatus1 == EE_RECEIVE_DATA) {
        status_wert = FLASH_EraseSector(PAGE0_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
        status_wert = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, EE_VALID_PAGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else {
        status_wert = EE_Format();
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      }
      break;
    case EE_RECEIVE_DATA:
      if (PageStatus1 == EE_VALID_PAGE) {
        x = -1;
        for (VarIdx = 0; VarIdx < EE_FLASH_MAX_ADR; VarIdx++) {
          if ((*(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VarIdx) {
            x = VarIdx;
          }
          if (VarIdx != x) {
            read_wert = UB_EE_FLASH_Read(VarIdx);
            if (read_wert >= 0) {
              data_wert = (uint16_t)(read_wert);
              status_wert = EE_VerifyPageFullWriteVariable(VarIdx, data_wert);
              if (status_wert != FLASH_COMPLETE) {
                return ERROR;
              }
            }
          }
        }
        status_wert = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, EE_VALID_PAGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
        status_wert = FLASH_EraseSector(PAGE1_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else if (PageStatus1 == EE_ERASED) {
        status_wert = FLASH_EraseSector(PAGE1_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
        status_wert = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, EE_VALID_PAGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else {
        status_wert = EE_Format();
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      }
      break;
    case EE_VALID_PAGE:
      if (PageStatus1 == EE_VALID_PAGE) {
        status_wert = EE_Format();
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else if (PageStatus1 == EE_ERASED) {
        status_wert = FLASH_EraseSector(PAGE1_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      } else {
        x = -1;
        for (VarIdx = 0; VarIdx < EE_FLASH_MAX_ADR; VarIdx++) {
          if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VarIdx) {
            x = VarIdx;
          }
          if (VarIdx != x) {
            read_wert = UB_EE_FLASH_Read(VarIdx);
            if (read_wert >= 0) {
              data_wert = (uint16_t)(read_wert);
              status_wert = EE_VerifyPageFullWriteVariable(VarIdx, data_wert);
              if (status_wert != FLASH_COMPLETE) {
                return ERROR;
              }
            }
          }
        }
        status_wert = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, EE_VALID_PAGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
        status_wert = FLASH_EraseSector(PAGE0_ID, VOLTAGE_RANGE);
        if (status_wert != FLASH_COMPLETE) {
          return ERROR;
        }
      }
      break;
    default:
      status_wert = EE_Format();
      if (status_wert != FLASH_COMPLETE) {
        return ERROR;
      }
      break;
  }

  return SUCCESS;
}

//--------------------------------------------------------------
// Daten aus virtuellem EEprom lesen
// VirtAddress : [0...EE_FLASH_MAX_ADR-1]
// Return_wert :
//  [0...65535] : wenn Inhalt von Adresse ausgelesen wurde
//  [-1]        : Adresse nicht gültig
//  [-2]        : Flash noch nicht initialisiert
//  [-3]        : Adresse wurde im Flash nicht gefunden
//--------------------------------------------------------------
int32_t
UB_EE_FLASH_Read(uint16_t VirtAddress)
{
  int32_t ret_wert = -3;
  uint16_t wert;
  uint16_t ValidPage;
  uint16_t AddressValue;
  uint32_t Address, PageStartAddress;

  if (VirtAddress >= EE_FLASH_MAX_ADR) {
    return -1;
  }

  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  if (ValidPage == EE_PAGE0) {
    PageStartAddress = (uint32_t)(PAGE0_BASE_ADDRESS);
    Address = (uint32_t)(PAGE0_END_ADDRESS - 1);
  } else if (ValidPage == EE_PAGE1) {
    PageStartAddress = (uint32_t)(PAGE1_BASE_ADDRESS);
    Address = (uint32_t)(PAGE1_END_ADDRESS - 1);
  } else {
    return -2;
  }

  ret_wert = -3;
  while (Address > (PageStartAddress + 2)) {
    AddressValue = (*(__IO uint16_t*)Address);

    if (AddressValue == VirtAddress) {
      wert = (*(__IO uint16_t*)(Address - 2));
      ret_wert = 0;

      break;
    } else {
      Address = Address - 4;
    }
  }

  if (ret_wert == 0) {
    // ausgelesenen Datenwert zurückgeben
    ret_wert = (int32_t)(wert);
  }

  return ret_wert;
}

//--------------------------------------------------------------
// Daten in virtuelles EEprom speichern
// VirtAddress : [0...EE_FLASH_MAX_ADR-1]
// Data : [0...65535]
// Return_wert :
//  [0]     : schreiben OK
//  [-1]    : Adresse nicht gültig
//  [-2]    : sonstiger Fehler
//--------------------------------------------------------------
int32_t
UB_EE_FLASH_Write(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t Status = EE_NO_ERROR;

  if (VirtAddress >= EE_FLASH_MAX_ADR) {
    return -1;
  }

  Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

  if (Status == EE_PAGE_FULL) {
    Status = EE_PageTransfer(VirtAddress, Data);
  }

  if (Status != FLASH_COMPLETE) {
    return -2;
  }

  return 0;
}

//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
FLASH_Status
EE_Format(void)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;

  FlashStatus = FLASH_EraseSector(PAGE0_ID, VOLTAGE_RANGE);

  if (FlashStatus != FLASH_COMPLETE) {
    return FlashStatus;
  }

  FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, EE_VALID_PAGE);

  if (FlashStatus != FLASH_COMPLETE) {
    return FlashStatus;
  }

  FlashStatus = FLASH_EraseSector(PAGE1_ID, VOLTAGE_RANGE);

  return FlashStatus;
}

//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
uint16_t
EE_FindValidPage(uint8_t Operation)
{
  uint16_t PageStatus0, PageStatus1;

  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);

  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  switch (Operation) {
    case WRITE_IN_VALID_PAGE:
      if (PageStatus1 == EE_VALID_PAGE) {
        if (PageStatus0 == EE_RECEIVE_DATA) {
          return EE_PAGE0;
        } else {
          return EE_PAGE1;
        }
      } else if (PageStatus0 == EE_VALID_PAGE) {
        if (PageStatus1 == EE_RECEIVE_DATA) {
          return EE_PAGE1;
        } else {
          return EE_PAGE0;
        }
      } else {
        return NO_VALID_PAGE;
      }
      break;
    case READ_FROM_VALID_PAGE:
      if (PageStatus0 == EE_VALID_PAGE) {
        return EE_PAGE0;
      } else if (PageStatus1 == EE_VALID_PAGE) {
        return EE_PAGE1;
      } else {
        return NO_VALID_PAGE;
      }
      break;
    default:
      return EE_PAGE0;
  }
}

//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
uint16_t
EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus;
  uint16_t ValidPage;
  uint32_t Address, PageEndAddress;

  ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

  if (ValidPage == EE_PAGE0) {
    Address = (uint32_t)(PAGE0_BASE_ADDRESS);
    PageEndAddress = (uint32_t)(PAGE0_END_ADDRESS - 1);
  } else if (ValidPage == EE_PAGE1) {
    Address = (uint32_t)(PAGE1_BASE_ADDRESS);
    PageEndAddress = (uint32_t)(PAGE1_END_ADDRESS - 1);
  } else {
    return NO_VALID_PAGE;
  }

  while (Address < PageEndAddress) {
    if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF) {
      FlashStatus = FLASH_ProgramHalfWord(Address, Data);
      if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
      }
      FlashStatus = FLASH_ProgramHalfWord(Address + 2, VirtAddress);
      return FlashStatus;
    } else {
      Address = Address + 4;
    }
  }

  return EE_PAGE_FULL;
}

//--------------------------------------------------------------
// interne Funktion
//--------------------------------------------------------------
uint16_t
EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint32_t NewPageAddress;
  uint16_t OldPageId;
  uint16_t ValidPage, VarIdx;
  uint16_t EepromStatus;
  int32_t read_wert;
  uint16_t data_wert;

  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  if (ValidPage == EE_PAGE1) {
    NewPageAddress = PAGE0_BASE_ADDRESS;
    OldPageId = PAGE1_ID;
  } else if (ValidPage == EE_PAGE0) {
    NewPageAddress = PAGE1_BASE_ADDRESS;
    OldPageId = PAGE0_ID;
  } else {
    return NO_VALID_PAGE;
  }

  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, EE_RECEIVE_DATA);
  if (FlashStatus != FLASH_COMPLETE) {
    return FlashStatus;
  }

  EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
  if (EepromStatus != FLASH_COMPLETE) {
    return EepromStatus;
  }

  for (VarIdx = 0; VarIdx < EE_FLASH_MAX_ADR; VarIdx++) {
    if (VarIdx != VirtAddress) {
      read_wert = UB_EE_FLASH_Read(VarIdx);
      if (read_wert >= 0) {
        data_wert = (uint16_t)(read_wert);
        EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, data_wert);
        if (EepromStatus != FLASH_COMPLETE) {
          return EepromStatus;
        }
      }
    }
  }

  FlashStatus = FLASH_EraseSector(OldPageId, VOLTAGE_RANGE);
  if (FlashStatus != FLASH_COMPLETE) {
    return FlashStatus;
  }

  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, EE_VALID_PAGE);
  if (FlashStatus != FLASH_COMPLETE) {
    return FlashStatus;
  }

  return FlashStatus;
}
