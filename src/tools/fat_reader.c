#include <stdio.h>
#include <stdint.h>

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

BootSector disk_BootSector;

bool Show_BPB(BootSector diskBootSector){



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


}



bool readBootRecord(FILE* diskImage){

    return fread(&disk_BootSector, sizeof(uint8_t), sizeof(disk_BootSector), diskImage) > 0; 
}



int main(int argc, char* arga[]){

    if (argc < 3){
        printf("Format is %s <disk_file> <file_name>\n", arga[0]);
        return -2;
    }

    FILE* diskImage = fopen(arga[1], "rb");

    if(diskImage == NULL){
        printf("Couldnt open the file %s...\n", arga[0]);
        return -1;
    }

    if(!readBootRecord(diskImage)){
        printf("Error reading the file..");
        return -3;
    }

    Show_BPB(disk_BootSector);


    return 0;


}