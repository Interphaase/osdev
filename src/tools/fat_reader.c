#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t bool;
#define true 1
#define false 0

typedef struct{

    //BIOS PARAMETER BLOCK for FAT12
    uint8_t BootJumpInstruction[3];
    uint8_t OemId[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATsNumber;
    uint16_t RootDirEntries;
    uint16_t TotalSectors;
    uint8_t MediaType;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t HeadsCount;
    uint32_t HiddenSectors;
    uint32_t LargeSectors;

    //EXTENDED BOOT RECORD
    uint8_t DriveNumber;
    uint8_t WindowsReserved;
    uint8_t Signature;
    uint8_t VolumeID[4];
    uint8_t VolumeLabel[11];
    uint8_t SystemID[8];

}__attribute__((packed)) BootSector;

typedef struct{
    uint8_t FileName[8];
    uint8_t FileExtension[3];
    uint8_t Attributes;
    uint16_t Reserved;
    uint16_t CreationTime;
    uint16_t CreationDate;
    uint16_t LastAccessDate;
    uint16_t Ignore;
    uint16_t LastWriteTime;
    uint16_t LastWriteDate;
    uint16_t FirstLogicalCluster;
    uint32_t FileSize;

}__attribute__((packed)) DirEntry;

BootSector disk_BootSector;
uint8_t* FATTable = NULL;
DirEntry* disk_RootEntry = NULL;

void Show_BPB(BootSector diskBootSector){
    printf("*************************************************\n");
    printf("***          BOOT SECTOR FAT12 READER V1.0    ***\n");
    printf("*************************************************\n");
    printf("READING BIOS PARAMETER BLOCK...\n");
    printf("OEM ID:                 |%.8s|\n",  diskBootSector.OemId);
    printf("Bytes per sector:       |%d|\n",    diskBootSector.BytesPerSector);
    printf("Sector per cluster:     |%d|\n",    diskBootSector.SectorsPerCluster);
    printf("Reserved sectors:       |%d|\n",    diskBootSector.ReservedSectors);
    printf("FATs count:             |%d|\n",    diskBootSector.FATsNumber);
    printf("Root DIR entries:       |%d|\n",    diskBootSector.RootDirEntries);
    printf("Total Sectors:          |%d|\n",    diskBootSector.TotalSectors);
    printf("Media Type:             |%d|\n",    diskBootSector.MediaType);
    printf("Sectors per FAT:        |%d|\n",    diskBootSector.SectorsPerFAT);
    printf("Sectors per Track:      |%d|\n",    diskBootSector.SectorsPerTrack);
    printf("Heads count:            |%d|\n",    diskBootSector.HeadsCount);
    printf("Hidden sectors:         |%d|\n",    diskBootSector.HiddenSectors);
    printf("Large sectors:          |%d|\n",    diskBootSector.LargeSectors);
    printf("READING EXTENDED BOOT RECORD...\n");
    printf("Drive Number:           |%d|\n",    diskBootSector.DriveNumber);
    printf("Windows Reserved bytes: |%d|\n",    diskBootSector.WindowsReserved);
    printf("Signature:              |%d|\n",    diskBootSector.Signature);
    printf("Volume ID:              |%d-%d-%d-%d|\n",    diskBootSector.VolumeID[0], diskBootSector.VolumeID[1], diskBootSector.VolumeID[2], diskBootSector.VolumeID[3]);
    printf("Volume Label:           |%.11s|\n", diskBootSector.VolumeLabel);
    printf("System ID:              |%.8s|\n",  diskBootSector.SystemID);
    
    return;

}

void Show_RootEntries(DirEntry* rootEntry, uint32_t rootEntriesCount){

    printf("Root Entries found on image: \n");
    for(int i = 0; i < rootEntriesCount; i++){
        printf("\nRoot Entry %d\n", i+1);
        printf("File name: |%.8s|\n", disk_RootEntry[i].FileName);
        printf("File extension: |%.3s|\n", disk_RootEntry[i].FileExtension);
        printf("File Attributes: |0x%x|\n", disk_RootEntry[i].Attributes);
        printf("Reserved: |0x%x|\n", disk_RootEntry[i].Reserved);
        printf("Creation Time: |%d|\n", disk_RootEntry[i].CreationTime);
        printf("Creation Date: |%d|\n", disk_RootEntry[i].CreationDate);
        printf("Last Access Date: |%d|\n", disk_RootEntry[i].LastAccessDate);
        printf("Ignore: |0x%x|\n", disk_RootEntry[i].Ignore);
        printf("Last Write Time: |%d|\n", disk_RootEntry[i].LastWriteTime);
        printf("Last Write Date: |%d|\n", disk_RootEntry[i].LastWriteDate);
        printf("First Logical Cluster: |%x|\n", disk_RootEntry[i].FirstLogicalCluster);
        printf("File size (bytes): |%d|\n", disk_RootEntry[i].FileSize);
    }
    return;
}



bool readBootRecord(FILE* diskImage){

    return fread(&disk_BootSector, sizeof(uint8_t), sizeof(disk_BootSector), diskImage) > 0; 
}

bool readSector(FILE* diskImage, uint32_t lba, uint32_t sector_count, void* buffer){

    bool read_state = true;
    read_state = read_state && (fseek(diskImage, lba*disk_BootSector.BytesPerSector, SEEK_SET) == 0);
    read_state = read_state && (fread(buffer, disk_BootSector.BytesPerSector, sector_count, diskImage) == sector_count);
    return read_state;

}

bool readFATTables(FILE* diskImage){
    FATTable = (uint8_t*) malloc(disk_BootSector.SectorsPerFAT*disk_BootSector.BytesPerSector);
    return readSector(diskImage, disk_BootSector.ReservedSectors, disk_BootSector.SectorsPerFAT, FATTable);
}

bool readRootDirectory(FILE* diskImage){
    uint32_t root_start_sector = disk_BootSector.ReservedSectors + disk_BootSector.FATsNumber*disk_BootSector.SectorsPerFAT;
    uint32_t root_length_bytes = disk_BootSector.RootDirEntries*32;
    uint32_t root_length_sectors = root_length_bytes/disk_BootSector.BytesPerSector;
    disk_RootEntry = (DirEntry*) malloc(root_length_sectors*disk_BootSector.BytesPerSector);
    return readSector(diskImage, root_start_sector, root_length_sectors, disk_RootEntry);
}



int main(int argc, char* arga[]){

    if (argc < 3){
        printf("Format is %s <disk_file> <file_name>\n", arga[0]);
        return -1;
    }

    FILE* diskImage = fopen(arga[1], "rb");

    if(diskImage == NULL){
        printf("Couldn't open the file %s...\n", arga[0]);
        return -2;
    }

    if(!readBootRecord(diskImage)){
        printf("Error reading the file %s..\n", arga[0]);
        return -3;
    }
    if(!readFATTables(diskImage)){
        printf("Error reading FAT table...\n");
        return -4;
    }
    if(!readRootDirectory(diskImage)){
        printf("Error reading Root Directory...\n");
        return -5;
    }

    Show_BPB(disk_BootSector);
    Show_RootEntries(disk_RootEntry, disk_BootSector.RootDirEntries);

    free(disk_RootEntry);
    free(FATTable);
    fclose(diskImage);
    return 0;


}