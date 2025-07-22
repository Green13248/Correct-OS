#include <stdint.h>
#include <stddef.h>
#include "include/util.h"
#include "include/VGA.h"
#include "include/kernel.h"
#include "include/ata_pio.h"
#include "include/SFS.h"

sfsFileEntry file_table[MAX_FILE_NUM];

bool load_in_file_table() {
    read_sector(1, (uint8_t *)file_table); //Sector one is where the file table is located
    for (int i = 0; i < 3; i++) {
        if (file_table[i].name[0] == 0) break;
    }
    return 0; //No error I hope :)
}

sfsFileEntry* find_file(const char *filename) {
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        if (file_table[i].name[0] == 0) {
            break; // Empty slot, stop here
        }
        if (strncmp(file_table[i].name, filename, 32) == 0) {
            return &file_table[i]; // Match
        }
    }
    return NULL;
}

bool read_file(const char *filename, uint8_t *buffer) {
    load_in_file_table();
    sfsFileEntry *file = find_file(filename);
    if (!file) {
        return 1;
    }
    uint32_t sectors = (file->size + 511) / 512;
    for (uint32_t i = 0; i < sectors; i++) {
        read_sector(file->start + i, buffer + (i * 512));
    }

    buffer[file->size] = '\0'; // IMPORTANT: null-terminate right after actual file content
    return true;
}

bool write_file(const char *filename, uint8_t* buffer) {
    uint8_t sector_buffer[512];
	read_sector(1, sector_buffer);
	dump_sector(sector_buffer);
    return 0;
}

void list_files() {
    sfsFileEntry file_table[32];
    read_sector(1, (uint8_t*)file_table);

    printf("\nDirectory:\n");

    for (int i = 0; i < 32; i++) {
        if (file_table[i].name[0] == 0) break; // End of entries

        printf(" - %s %d %d\n", 
               file_table[i].name, 
               file_table[i].size, 
               file_table[i].start);
    }
}