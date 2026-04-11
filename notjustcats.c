#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BUFFERSIZE 1024
#define SECTORSIZE 512

#define FAT1START 1
#define FAT2START 10
#define ROOTDIRSTART 19
#define DATASTART 33

typedef struct{
    char fileName[8];
    char extension[3];
    uint8_t Attributes;
    uint16_t Reserved;
    uint16_t creationTime;
    uint16_t lastAccessDate;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t firstLogicalCluster;
    uint32_t fileSize;
}__attribute__((packed)) dirEntry;

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Correct Usage: ./notjustcats <imagefilename> <output_directory>\n");
        return 0;
    }
    char *imageFile = argv[1];
    char *outputDirectory = argv[2];

    FILE *fp = fopen(imageFile, "r");
    if (fp == NULL){
        perror("fopen");
        return 0;
    }

    dirEntry *entry = malloc(sizeof(dirEntry));
    
    int i = 0;
    int location = ROOTDIRSTART;
    while (location < DATASTART){
        fseek(fp, (ROOTDIRSTART * SECTORSIZE) + i, SEEK_SET); //Get to the first entry of root dir
        fread(entry, sizeof(dirEntry), 1, fp);

        if (entry -> fileName[0] == 0x00) break; //No more entries
        if (entry -> Attributes == 0x0F) continue; //Ignore Long File Names
        if (entry -> Attributes & 0x08) continue; //volume label

        if (entry -> fileName[0] == 0xE5){ //delete File

        }else if (entry -> Attributes & 0x10){ //subdirectory
        
        }else{ //normal file
            printf("FILE\tNORMAL\t%.8s.%.3s\t%d\n", entry -> fileName, entry -> extension, entry -> fileSize);
        }
        i = i + 16;
        location = location + 16;
    }

    fclose(fp);
    return 0;
}