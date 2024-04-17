#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t bool;
#define true 1
#define false 0

typedef struct
{

    // BIOS PARAMETER BLOCK for FAT12
    uint8_t bootJumpInstruction[3];
    uint8_t oemId[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t FAT_number;
    uint16_t rootDirEntries;
    uint16_t totalSectors;
    uint8_t mediaType;
    uint16_t sectorsPerFAT;
    uint16_t sectorsPerTrack;
    uint16_t headsCount;
    uint32_t hiddenSectors;
    uint32_t largeSectors;

    // EXTENDED BOOT RECORD
    uint8_t driveNumber;
    uint8_t windowsReserved;
    uint8_t signature;
    uint8_t volumeID[4];
    uint8_t volumeLabel[11];
    uint8_t systemID[8];

} __attribute__((packed)) BootSector;

typedef struct
{
    uint8_t fileName[8];
    uint8_t fileExtension[3];
    uint8_t attributes;
    uint16_t reserved;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t ignore;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t firstLogicalCluster;
    uint32_t fileSize;

} __attribute__((packed)) DirEntry;

BootSector disk_BootSector;
uint8_t* FAT_Table = NULL;
DirEntry* disk_RootEntry = NULL;

void dump_fatTable(uint8_t* fatTable){
    printf("\nDUMPING FAT Table contents...\n");
    for(int i = 0; i < disk_BootSector.sectorsPerFAT*disk_BootSector.bytesPerSector-1; i++){
        uint16_t entry_buffer;
        entry_buffer = ((uint16_t)fatTable[i] << 8) | fatTable[i+1];
        if(i % 2 == 0){//even
            entry_buffer = entry_buffer >> 4;
        }
        else{//odd
            entry_buffer = entry_buffer & 0b0000111111111111;
        }
        printf("%dx%x ",i, entry_buffer);
        if(i % 20 == 0 && i != 0){
            printf("\n");
        }
    }
    printf("\n");
}

void show_BPB(BootSector diskBootSector)
{
    printf("*************************************************\n");
    printf("***          BOOT SECTOR FAT12 READER V1.0    ***\n");
    printf("*************************************************\n");
    printf("READING BIOS PARAMETER BLOCK...\n");
    printf("OEM ID:                 |%.8s|\n", diskBootSector.oemId);
    printf("Bytes per sector:       |%d|\n", diskBootSector.bytesPerSector);
    printf("Sector per cluster:     |%d|\n", diskBootSector.sectorsPerCluster);
    printf("Reserved sectors:       |%d|\n", diskBootSector.reservedSectors);
    printf("FATs count:             |%d|\n", diskBootSector.FAT_number);
    printf("Root DIR entries:       |%d|\n", diskBootSector.rootDirEntries);
    printf("Total Sectors:          |%d|\n", diskBootSector.totalSectors);
    printf("Media Type:             |%d|\n", diskBootSector.mediaType);
    printf("Sectors per FAT:        |%d|\n", diskBootSector.sectorsPerFAT);
    printf("Sectors per Track:      |%d|\n", diskBootSector.sectorsPerTrack);
    printf("Heads count:            |%d|\n", diskBootSector.headsCount);
    printf("Hidden sectors:         |%d|\n", diskBootSector.hiddenSectors);
    printf("Large sectors:          |%d|\n", diskBootSector.largeSectors);
    printf("READING EXTENDED BOOT RECORD...\n");
    printf("Drive Number:           |%d|\n", diskBootSector.driveNumber);
    printf("Windows Reserved bytes: |%d|\n", diskBootSector.windowsReserved);
    printf("Signature:              |%d|\n", diskBootSector.signature);
    printf("Volume ID:              |%d-%d-%d-%d|\n", diskBootSector.volumeID[0], diskBootSector.volumeID[1], diskBootSector.volumeID[2], diskBootSector.volumeID[3]);
    printf("Volume Label:           |%.11s|\n", diskBootSector.volumeLabel);
    printf("System ID:              |%.8s|\n", diskBootSector.systemID);

    return;
}

void show_RootEntries(DirEntry* rootEntry, uint32_t rootEntriesCount)
{

    printf("Root Entries found on image: \n");
    for (int i = 0; i < rootEntriesCount; i++)
    {
        if (disk_RootEntry[i].fileName[0] == 0)
        {
            break;
        }
        uint16_t creation_time_hours, creation_time_minutes, creation_time_seconds;
        uint16_t last_write_time_hours, last_write_time_minutes, last_write_time_seconds;
        uint16_t creation_date_year, creation_date_month, creation_date_day;
        uint16_t last_access_year, last_access_month, last_access_day;
        uint16_t last_write_year, last_write_month, last_write_day;

        creation_time_seconds = (disk_RootEntry[i].creationTime & 31) * 2;
        creation_time_minutes = (disk_RootEntry[i].creationTime >> 5) & 63;
        creation_time_hours = (disk_RootEntry[i].creationTime >> 11) & 31;

        last_write_time_seconds = (disk_RootEntry[i].lastWriteTime & 31) * 2;
        last_write_time_minutes = (disk_RootEntry[i].lastWriteTime >> 5) & 63;
        last_write_time_hours = (disk_RootEntry[i].lastWriteTime >> 11) & 31;

        creation_date_day = (disk_RootEntry[i].creationDate) & 31;
        creation_date_month = (disk_RootEntry[i].creationDate >> 5) & 63;
        creation_date_year = ((disk_RootEntry[i].creationDate >> 9) & 127)+ 1980;

        last_access_day = (disk_RootEntry[i].lastAccessDate) & 31;
        last_access_month = (disk_RootEntry[i].lastAccessDate >> 5) & 63;
        last_access_year = ((disk_RootEntry[i].lastAccessDate >> 9) & 127)+ 1980;

        last_write_day = (disk_RootEntry[i].lastWriteDate) & 31;
        last_write_month = (disk_RootEntry[i].lastWriteDate >> 5) & 63;
        last_write_year = ((disk_RootEntry[i].lastWriteDate >> 9) & 127)+ 1980;

        printf("\nRoot Entry %d\n", i + 1);
        printf("File name: |%.8s|\n", disk_RootEntry[i].fileName);
        printf("File extension: |%.3s|\n", disk_RootEntry[i].fileExtension);
        printf("File Attributes: |0x%x|\n", disk_RootEntry[i].attributes);
        printf("Reserved: |0x%x|\n", disk_RootEntry[i].reserved);
        printf("Creation Time: |%d:%d:%d|\n", creation_time_hours, creation_time_minutes, creation_time_seconds);
        printf("Creation Date: |%d.%d.%d|\n", creation_date_day, creation_date_month, creation_date_year);
        printf("Last Access Date: |%d.%d.%d|\n", last_access_day, last_access_month, last_access_year);
        printf("Ignore: |0x%x|\n", disk_RootEntry[i].ignore);
        printf("Last Write Time: |%d:%d:%d|\n", last_write_time_hours, last_write_time_minutes, last_write_time_seconds);
        printf("Last Write Date: |%d.%d.%d|\n", last_write_day, last_write_month, last_write_year);
        printf("First Logical Cluster: |%x|\n", disk_RootEntry[i].firstLogicalCluster);
        printf("File size (bytes): |%d|\n", disk_RootEntry[i].fileSize);
    }
    return;
}

bool readBootRecord(FILE* diskImage)
{

    return fread(&disk_BootSector, sizeof(uint8_t), sizeof(disk_BootSector), diskImage) > 0;
}

bool readSector(FILE* diskImage, uint32_t lba, uint32_t sector_count, void* buffer)
{

    bool read_state = true;
    read_state = read_state && (fseek(diskImage, lba * disk_BootSector.bytesPerSector, SEEK_SET) == 0);
    read_state = read_state && (fread(buffer, disk_BootSector.bytesPerSector, sector_count, diskImage) == sector_count);
    return read_state;
}

bool readFATTables(FILE* diskImage)
{
    FAT_Table = (uint8_t*)malloc(disk_BootSector.sectorsPerFAT * disk_BootSector.bytesPerSector);
    return readSector(diskImage, disk_BootSector.reservedSectors, disk_BootSector.sectorsPerFAT, FAT_Table);
}

bool readRootDirectory(FILE* diskImage)
{
    uint32_t root_start_sector = disk_BootSector.reservedSectors + disk_BootSector.FAT_number * disk_BootSector.sectorsPerFAT;
    uint32_t root_length_bytes = disk_BootSector.rootDirEntries * 32;
    uint32_t root_length_sectors = root_length_bytes / disk_BootSector.bytesPerSector;
    disk_RootEntry = (DirEntry*)malloc(root_length_sectors * disk_BootSector.bytesPerSector);
    return readSector(diskImage, root_start_sector, root_length_sectors, disk_RootEntry);
}

int main(int argc, char* argv[])
{

    if (argc < 3)
    {
        printf("Format is %s <disk_file> <file_name>\n", argv[0]);
        return -1;
    }

    FILE* diskImage = fopen(argv[1], "rb");

    if (diskImage == NULL)
    {
        printf("Couldn't open the file %s...\n", argv[0]);
        return -2;
    }

    if (!readBootRecord(diskImage))
    {
        printf("Error reading the file %s..\n", argv[0]);
        return -3;
    }
    if (!readFATTables(diskImage))
    {
        printf("Error reading FAT table...\n");
        return -4;
    }
    if (!readRootDirectory(diskImage))
    {
        printf("Error reading Root Directory...\n");
        return -5;
    }

    show_BPB(disk_BootSector);
    show_RootEntries(disk_RootEntry, disk_BootSector.rootDirEntries);
    dump_fatTable(FAT_Table);

    free(disk_RootEntry);
    free(FAT_Table);
    fclose(diskImage);
    return 0;
}