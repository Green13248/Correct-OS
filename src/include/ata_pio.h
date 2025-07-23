#include <stdint.h>

bool read_sector(uint32_t lba, uint8_t* buffer);
bool write_sector(uint32_t lba, uint8_t* buffer);