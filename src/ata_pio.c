#include <stdint.h>
#include "include/util.h"
#include "include/VGA.h"

#define DATA_REGISTER 0x1F0
#define ERROR_REGISTER 0x1F1
#define FEATURES_REGISTER 0x1F1
#define SECTOR_COUNT_REGISTER 0x1F2
#define SECTOR_NUMBER_REGISTER 0x1F3 //low LBA
#define CYLINDER_LOW_REGISTER 0x1F4 //mid LBA
#define CYLINDER_HIGH_REGISTER 0x1F5 //High LBA
#define DRIVE_REGISTER 0x1F6
#define STATUS_REGISTER 0x1F7
#define COMMAND_REGISTER 0x1F7

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ( "inw %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

//LBA 28 only 128 gb, write a 48 bute for 128 pb
//Read sector into buffer okay
void read_sector(uint32_t lba, uint8_t* buffer) {
    outPortB(0x1F6, 0xE0);
    outPortB(0x1F1, 0x00); //Null value
    outPortB(0x1F2, (unsigned char) 1);
    outPortB(SECTOR_NUMBER_REGISTER, (unsigned char)(lba)); //low 8 bits
    outPortB(CYLINDER_LOW_REGISTER, (unsigned char)(lba >> 8)); //mid 8 bits
    outPortB(CYLINDER_HIGH_REGISTER, (unsigned char)(lba >> 16)); //high 8 bits
    outPortB(COMMAND_REGISTER, 0x20);

    while (!(inPortB(STATUS_REGISTER) & 0x08)); //Wait for drive to respond (add a timeout to this and set a drive error?)
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(DATA_REGISTER);
        buffer[i * 2 + 0] = data & 0xFF;
        buffer[i * 2 + 1] = (data >> 8) & 0xFF;
    }
}

void outsb(uint16_t port, const void* addr, uint32_t count) {
    asm volatile ("rep outsb"
                  : "+S"(addr), "+c"(count)
                  : "d"(port));
}

void write_sector(uint32_t lba, uint8_t* buffer) {
    outPortB(0x1F6, 0xE0 | ((lba >> 24) & 0xF));  // LBA + master drive
    outPortB(0x1F2, 1); //One can be changed to num sectors to write but I shall keep it like this for now
    outPortB(0x1F3, lba); //low
    outPortB(0x1F4, lba >> 8); //mid
    outPortB(0x1F5, lba >> 16); //high
    outPortB(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outPortB(0x1F7, 0x30);  // WRITE SECTORS or issue write command
    //wait for drive ready, write 512 bytes
    for (int i = 0; i < 512; i += 2) {
        uint16_t word = buffer[i] | (buffer[i+1] << 8);
        outPortW(0x1F0, word);
    }
    outPortB(0x1F7, 0xE7);  // FLUSH CACHE
}