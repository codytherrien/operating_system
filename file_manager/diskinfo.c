/* 
Cody Therrien
V00747104
CSC 360
Assignment 3
Disk Info
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Folder.h"
#include "disk_functs.h"

// Updates OS name
void get_os_name(char* file_p, char* os_name) {
    for(int i=0; i<8; i++) {
        os_name[i] = file_p[i+3];
    }
}

// Updates disk label
void get_label(char* file_p, char* disk_label) {
    for(int i=0; i<8; i++) {
        disk_label[i] = file_p[i+43];
    }

    if(disk_label[0] == ' ') {
        file_p += SECTOR_SZ*19;
        while(file_p[0]) {
            if(file_p[11] == 8) {
                for(int i=0; i<8; i++) disk_label[i] = file_p[i];
            }
            file_p += 32;
        }
    }
}

// Updates the nummber of files on the disk
void get_num_files(char* file_p, int* num_files) {
    char* og_p = file_p;
    file_p += SECTOR_SZ*19;
    struct Folder* que = NULL;
    struct Folder* folder = malloc(sizeof(struct Folder));
    folder->file_p = file_p;
    folder->name = "Root";
    folder->next = NULL;

    add_folder(&que, folder);

    while(que != NULL) {
        folder = pop_folder(&que);
        file_p = folder->file_p;

        while(file_p[0]) {
            if((file_p[11] & 0b00010000) == 0b00010000) {
                folder->file_p = og_p + SECTOR_SZ*(31 + (file_p[26] + (file_p[27] << 8)));  
                folder->name = NULL;
                folder->next = NULL;
                add_folder(&que, folder);
            }

            if ((file_p[11] & 0b00000010) == 0 && (file_p[11] & 0b00001000) == 0 && (file_p[11] & 0b00010000) != 0b00010000) {
                *num_files += 1;
            }
            file_p += 32;
        }
    }
}

// Updates the number of FAT copies
void get_fat_copies(char* file_p, int* num_fat_copies) {
    *num_fat_copies = file_p[16];
}

// Updates the number of sectors per FAT
void get_sectors_per_fat(char* file_p, int* sectors_per_fat) {
    *sectors_per_fat = file_p[22] + (file_p[23] << 8); // Bitshift by 1 byte
}



int main(int argc, char* argv[]) {
    int file;
    int read;
    char* file_p;
    struct stat disk;
    char* os_name = malloc(sizeof(char));
    char* disk_label = malloc(sizeof(char));
    int disk_size;
    int free_space = 0;
    int num_files;
    int num_fat_copies;
    int sectors_per_fat;

    if(argc < 2) {
        printf("ERROR: Include disk image as argument\n");
        exit(-1);
    }

    file = open(argv[1], O_RDWR);
    if(file < 0) {
        printf("ERROR: Disk read error\n");
        exit(-1);
    }

    read = fstat(file, &disk);
    if(read < 0) {
        printf("ERROR: Failed to write to buffer\n");
        exit(-1);
    }

    file_p = mmap(NULL, disk.st_size,  PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if(file_p == MAP_FAILED) {
        printf("ERROR: Memeory mapping failed\n");
        exit(-1);
    }

    get_os_name(file_p, os_name);
    get_label(file_p, disk_label);
    get_size(file_p, &disk_size);
    get_free_space(file_p, &free_space, disk_size);
    get_num_files(file_p, &num_files);
    get_fat_copies(file_p, &num_fat_copies);
    get_sectors_per_fat(file_p, &sectors_per_fat);

    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", disk_label);
    printf("Total side of the disk: %d bytes\n", disk_size);
    printf("Free size of the disk: %d bytes\n", free_space);
    printf("==============\n");
    printf("The number of files on the image: %d\n", num_files);
    printf("==============\n");
    printf("Number of FAT compies: %d\n", num_fat_copies);
    printf("Sectors per FAT: %d\n", sectors_per_fat);
}