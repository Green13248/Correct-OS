import os
import struct

SECTOR_SIZE = 512
FILE_ENTRY_SIZE = 40
MAGIC = 0x53465331  # "SFS1"
TABLE_SECTOR = 1    # File table starts at sector 1
DATA_SECTOR_START = 2  # File contents start here

def pad_to_sector(data):
    if len(data) % SECTOR_SIZE == 0:
        return data
    padding = SECTOR_SIZE - (len(data) % SECTOR_SIZE)
    return data + b'\x00' * padding

def inject_files(disk_path, file_list):
    # Calculate number of sectors needed
    file_entries = []
    current_sector = DATA_SECTOR_START

    # Process each file
    file_data_blocks = []
    for path in file_list:
        name = os.path.basename(path).encode("utf-8")
        name = name[:31] + b'\x00'  # Ensure null-terminated 32 bytes
        name = name.ljust(32, b'\x00')

        with open(path, 'rb') as f:
            content = f.read()
        size = len(content)
        padded = pad_to_sector(content)
        sectors_needed = len(padded) // SECTOR_SIZE

        entry = {
            "name": name,
            "start": current_sector,
            "size": size
        }

        file_entries.append(entry)
        file_data_blocks.append(padded)
        current_sector += sectors_needed

    # Superblock
    superblock = struct.pack("<IIII",
        MAGIC,
        len(file_entries),
        TABLE_SECTOR,
        DATA_SECTOR_START
    )
    superblock = superblock.ljust(SECTOR_SIZE, b'\x00')

    # File table
    file_table = b""
    for entry in file_entries:
        file_table += struct.pack("<32sII", entry["name"], entry["start"], entry["size"])
    file_table = file_table.ljust(SECTOR_SIZE, b'\x00')

    # Write to disk image
    with open(disk_path, 'r+b') as img:
        img.seek(0)
        img.write(superblock)
        img.write(file_table)
        for block in file_data_blocks:
            img.write(block)

    print(f"Injected {len(file_entries)} file(s) into {disk_path}")

# Example usage
if __name__ == "__main__":
    inject_files("hdd.img", ["hello.txt", "data.bin", "file.txt", "program.bin"])