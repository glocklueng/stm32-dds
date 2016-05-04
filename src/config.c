#include "config.h"

#include "crc.h"
#include "eeprom.h"

#define CONFIG_EEPROM eeprom_block1

static const struct config default_config = {
  .ethernet =
    {
      .address = { 172, 31, 10, 50 },
      .submask = { 255, 255, 255, 0 },
      .gateway = { 172, 31, 10, 1 },
    },
};

static uint32_t calculate_config_crc(const struct config*);

const struct config*
config_get()
{
  uint32_t* crc_stored = eeprom_get(CONFIG_EEPROM, 0);

  const struct config* conf = eeprom_get(CONFIG_EEPROM, sizeof(uint32_t));

  uint32_t crc_calc = calculate_config_crc(conf);

  /* if the crc check fails we reset the config back to the default
   * configuration */
  if (*crc_stored != crc_calc) {
    config_write(&default_config);
  }

  return conf;
}

void
config_write(const struct config* conf)
{
  eeprom_erase(CONFIG_EEPROM);

  uint32_t crcsum = calculate_config_crc(conf);

  eeprom_write(CONFIG_EEPROM, 0, &crcsum, sizeof(uint32_t));
  eeprom_write(CONFIG_EEPROM, sizeof(uint32_t), conf, sizeof(struct config));
}

static uint32_t
calculate_config_crc(const struct config* conf)
{
  crc_init();

  return crc((const uint32_t*)conf, sizeof(struct config) / sizeof(uint32_t));
}
