#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

#define BUFFERSIZE 1024
#define SECTORSIZE 512
#define PATHBUFFER 64

#define FAT1START 1
#define FAT2START 10
#define ROOTDIRSTART 19
#define DATASTART 33

#define NAME 1
#define EXT 2

#define NORMAL 1
#define DELETED -1

int numFiles = 0; //Keep track of number of files for naming purposes

typedef struct{
    char *fileName;
    int fileSize;
    char *Data;
} fileStruct;

typedef struct{
    char fileName[8];
    char extension[3];
    uint8_t Attributes;
    uint8_t Reserved[2];
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t randomTwoBytes;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t firstLogicalCluster;
    uint32_t fileSize;
}__attribute__((packed)) dirEntry;

char *nameFormat(char *string, int TYPE){
    char *buf = calloc(1, PATHBUFFER);
    int len;
    len = (TYPE == NAME) ? 8 : 3;
    memcpy(buf, string, len);

    int i = 0;
    while (i < len && buf[i] != ' '){
        i++;
    }
    buf[i] = '\0';

    return buf;
}

void printFormat(dirEntry *entry, char *currentFilePath, int fileStatus){
    char *name = nameFormat(entry->fileName, NAME);
    char *ext = nameFormat(entry->extension, EXT);
    char *status = "NORMAL";
    if (fileStatus == DELETED){
        name[0] = '_';
        status = "DELETED";
    }
    if (strlen(ext) > 0)
        printf("FILE\t%s\t%s%s.%s\t%d\n", status, currentFilePath, name, ext, entry->fileSize);
    else
        printf("FILE\t%s\t%s%s\t%d\n", status, currentFilePath, name, entry->fileSize);
    free(name);
    free(ext);
}

void createFile(dirEntry *entry, uint16_t *FAT, char *Data, char *outDir){
    char fileName[BUFFERSIZE];
    char *ext = nameFormat(entry -> extension, EXT);
    if (strlen(ext) > 0){
        snprintf(fileName, sizeof(fileName), "%s/file%d.%s", outDir, numFiles++, ext);
    }else{
        snprintf(fileName, sizeof(fileName), "%s/file%d", outDir, numFiles++);
    }

    FILE *f = fopen(fileName, "w");
    int cluster = entry -> firstLogicalCluster;
    int sector = cluster - 2;
    char *data = Data + (sector * SECTORSIZE);

    if (entry -> fileSize <= SECTORSIZE){
        fwrite(data, 1, entry -> fileSize, f);
    }else{
        int sizeRemaining = entry -> fileSize;
        int copySize;
        while (cluster < 0xFF8){
            copySize = (sizeRemaining < SECTORSIZE) ? sizeRemaining : SECTORSIZE;
            fwrite(data, 1, copySize, f);
            cluster = FAT[cluster];
            sector = cluster - 2;
            data = Data + (sector * SECTORSIZE);
            sizeRemaining -= copySize;
        }
    }
    fclose(f);
}

void createDeletedFile(dirEntry *entry, uint16_t *FAT, char *Data, char *outDir){
    char fileName[BUFFERSIZE];
    char *ext = nameFormat(entry -> extension, EXT);
    if (strlen(ext) > 0){
        snprintf(fileName, sizeof(fileName), "%s/file%d.%s", outDir, numFiles++, ext);
    }else{
        snprintf(fileName, sizeof(fileName), "%s/file%d", outDir, numFiles++);
    }

    FILE *f = fopen(fileName, "w");
    int cluster = entry -> firstLogicalCluster;
    int sector = cluster - 2;
    char *data = Data + (sector * SECTORSIZE);
    if (entry -> fileSize <= SECTORSIZE){
        fwrite(data, 1, entry -> fileSize, f);
    }else{
        int sizeRemaining = entry -> fileSize;
        int copySize;
        while (FAT[cluster] == 0x000){
            copySize = (sizeRemaining < SECTORSIZE) ? sizeRemaining : SECTORSIZE;
            fwrite(data, 1, copySize, f);
            cluster++;
            sector = cluster - 2;
            data = Data + (sector * SECTORSIZE);
            sizeRemaining -= copySize;
        }
    }
    fclose(f);
}

void clusterFormat(uint8_t value1, uint8_t value2, uint8_t value3, uint16_t *out1, uint16_t *out2){
    *out1 = ((value2 & 0x0F) << 8) | value1;
    *out2 = (value3 << 4) | (value2 >> 4);
}

void recFATHandler(dirEntry *entry, uint16_t *FAT, char *Data, char *outDir, char *currentFilePath){
    if (entry -> fileName[0] == 0x00) return; //Base Case
    else if((unsigned char)entry -> fileName[0] == 0xE5){
        //Deleted File
        if (entry -> Attributes & 0x10){// Deleted Directory
            char buf[PATHBUFFER];
            char *name = nameFormat(entry -> fileName, NAME);
            char *ext = nameFormat(entry -> extension, EXT);
            name[0] = '_';
            if (strlen(ext) > 0) snprintf(buf, sizeof(buf), "%s%s.%s/", currentFilePath, name, ext);
            else snprintf(buf, sizeof(buf), "%s%s/", currentFilePath, name);
            int cluster = entry -> firstLogicalCluster;
            int sector = cluster - 2;
            dirEntry *nextDir = (dirEntry *)(Data + (sector * SECTORSIZE));
            recFATHandler(nextDir, FAT, Data, outDir, buf);
            recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath);    
        }else{
            int cluster = entry -> firstLogicalCluster;
            if (FAT[cluster] == 0){
                    printFormat(entry, currentFilePath, DELETED);
                    createDeletedFile(entry, FAT, Data, outDir);
            }
            recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath);            
        }
    }else if(entry -> Attributes & 0x10){
        //Subdirectory
        if (entry -> fileName[0] == '.') return recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath); 
        char buf[PATHBUFFER];
        if (strlen(nameFormat(entry -> extension, EXT)) > 0)snprintf(buf, sizeof(buf), "%s%s.%s/", currentFilePath, nameFormat(entry -> fileName, NAME), nameFormat(entry -> extension, EXT));
        else snprintf(buf, sizeof(buf), "%s%s/", currentFilePath, nameFormat(entry -> fileName, NAME));

        int cluster = entry -> firstLogicalCluster;
        int sector = cluster - 2;
        dirEntry *nextDir = (dirEntry *)(Data + (sector * SECTORSIZE));
        recFATHandler(nextDir, FAT, Data, outDir, buf);
        recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath);
    }else if(entry -> Attributes == 0x0F)return recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath);
    else{
        //Handle the File
        createFile(entry, FAT, Data, outDir);
        printFormat(entry, currentFilePath, NORMAL);
        return recFATHandler(entry + 1, FAT, Data, outDir, currentFilePath); 
    }
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Correct Usage: ./notjustcats <imagefilename> <output_directory>\n");
        return 0;
    }
    char *imageFile = argv[1];
    char *outputDirectory = argv[2];

    int fd = open(imageFile, O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    fstat(fd,&sb);
    char *filemappedpage = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    
    char *FAT1 = filemappedpage + (SECTORSIZE * FAT1START);
    char *rootDir = filemappedpage + (SECTORSIZE * ROOTDIRSTART);
    char *Data = filemappedpage + (SECTORSIZE * DATASTART);

    //Set up Fat
    int fatEntries = ((SECTORSIZE * (FAT2START - FAT1START)) * 2) / 3;
    uint16_t *formattedFAT = malloc(fatEntries * sizeof(uint16_t));
    uint8_t *firstClusterByte = (uint8_t *)FAT1;
    uint16_t out1, out2;
    int i = 0;
    while ((firstClusterByte + 2) < (uint8_t *)(FAT1) + (SECTORSIZE * (FAT2START - FAT1START))){
        clusterFormat(*firstClusterByte, *(firstClusterByte + 1), *(firstClusterByte + 2), &out1, &out2);
        formattedFAT[i] = out1;
        formattedFAT[i + 1] = out2;

        firstClusterByte += 3;
        i += 2;
    }

    //Begin Parse
    dirEntry *entry = (dirEntry *)rootDir;
    recFATHandler(entry, formattedFAT, Data, outputDirectory, "/");
    return 0;
}