#pragma

typedef struct {
    char name[32];
    uint32_t start;
    uint32_t size;
} __attribute__((packed)) sfsFileEntry;

#define MAX_FILE_NUM 32

struct SFS_Superblock {
    uint32_t magic;
    uint32_t file_count;
    uint32_t table_start;
    uint32_t data_start;
};

bool load_in_file_table();
sfsFileEntry* find_file(const char *filename);
bool read_file(const char *filename, uint8_t *buffer);
void list_files();
bool write_file(const char *filename, uint8_t* buffer);