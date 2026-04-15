#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFFERSIZE 1024
#define SECTORSIZE 512

#define FAT1START 1
#define FAT2START 10
#define ROOTDIRSTART 19
#define DATASTART 33

#define NAME 1
#define EXT 2

typedef struct{
    char fileName[8];
    char extension[3];
    uint8_t Attributes;
    uint16_t Reserved[2];
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t firstLogicalCluster;
    uint32_t fileSize;
}__attribute__((packed)) dirEntry;

char *nameFormat(char *string, int TYPE){
    char *buf = calloc(1, 64);
    int len;
    len = (TYPE == NAME) ? 8 : 3;
    memcpy(buf, string, len);

    int i = 0;
    while (buf[i] != ' ' && i < len){
        i++;
    }
    buf[i] = '\0';

    return buf;
}

void printFormat(dirEntry *entry){
    printf("FILE\tNORMAL\t%.8s.%.3s\t%d\n", nameFormat(entry -> fileName, NAME), nameFormat(entry -> extension, EXT), entry -> fileSize);
}

void clusterFormat(uint8_t value1, uint8_t value2, uint8_t value3, uint16_t *out1, uint16_t *out2){
    *out1 = ((value2 & 0x0F) << 8) | value1;
    *out2 = (value3 << 4) | (value2 >> 4);
}

void subdirectoryHandler(dirEntry *entry){
    uint16_t cluster = entry -> firstLogicalCluster;
    
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Correct Usage: ./notjustcats <imagefilename> <output_directory>\n");
        return 0;
    }
    char *imageFile = argv[1];
    //char *outputDirectory = argv[2];

    int fd = open(imageFile, O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    fstat(fd,&sb);
    char *filemappedpage = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    
    char *FAT1 = filemappedpage + (SECTORSIZE * FAT1START);
    //char *FAT2 = filemappedpage + (SECTORSIZE * FAT2START);
    char *rootDir = filemappedpage + (SECTORSIZE * ROOTDIRSTART);
    //char *Data = filemappedpage + (SECTORSIZE * DATASTART);

    dirEntry *entry = NULL;
    printf("sizeof(dirEntry) = %lu\n", sizeof(dirEntry));
    int i = 0;
    while (i < 224){ //All possible root directory entries
        entry = (dirEntry *)(rootDir + (i * sizeof(dirEntry)));
        if (entry -> fileName[0] == 0x00) break;
        else if (entry -> Attributes == 0x0F) i++;
        else if(entry -> fileName[0] == 0xE5){
            //Deleted File
            i++;
        }else if(entry -> Attributes & 0x10){
            //Subdirectory
            subdirectoryHandler(entry);
            i++;
        }else{
            //Actual File
            printFormat(entry);
            i++;
        }
    }
    return 0;
}