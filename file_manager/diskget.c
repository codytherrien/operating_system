/* 
Cody Therrien
V00747104
CSC 360
Assignment 3
Disk Get
*/

#include <stdio.h>
#include "disk_functs.h"

// Returns the file size of the file name passed in
int get_file_size(char* file_p, char* file_name) {
    file_p += SECTOR_SZ*19; 

    while(file_p[0]) {
        char* curr_file_name = malloc(sizeof(char));
        int i;

        for(i=0; i<8; i++) {
            if(file_p[i] == ' ') break;
            curr_file_name[i] = tolower(file_p[i]);
        }

        curr_file_name[i] = '.';
        i++;
        for(int j=8; j<11; j++) {
            if(file_p[j] == ' ') break;
            curr_file_name[i] = tolower(file_p[j]);
            i++;
        }

        if(strcmp(curr_file_name, file_name) == 0) {
            return file_p[28] + (file_p[29] << 8) + (file_p[30] << 16) + (file_p[31] << 24);
        }
        file_p += 32; 
    }

    return -1;
}

// maps memory space for the copied file
char* get_new_pointer(int file_size, char* name) {
    char* new_p;
    int file2;
    file2 = open(name, O_RDWR | O_CREAT, 0666);
    if(file2 < 0) {
        printf("ERROR: Failed to open file\n");
        exit(-1);
    }
    if(lseek(file2, file_size-1, SEEK_SET) == -1) {
        printf("ERROR: Failed to see to file end\n");
        exit(-1);
    }
    if(write(file2, "", 1) != 1) {
        printf("ERROR: Failed to write all bytes\n");
        exit(-1);
    }

    new_p = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, file2, 0);
    if(new_p == MAP_FAILED) {
        printf("ERROR: Failed to map file to memory\n");
        exit(-1);
    }
    return new_p;
}

// locates first sector of file
int get_start_sector(char* file_name, char* file_p) {
    file_p += SECTOR_SZ*19;

    while(file_p[0]) {
        char* curr_file_name = malloc(sizeof(char));
        int i;

        for(i=0; i<8; i++) {
            if(file_p[i] == ' ') break;
            curr_file_name[i] = tolower(file_p[i]);
        }

        curr_file_name[i] = '.';
        i++;
        for(int j=8; j<11; j++) {
            if(file_p[j] == ' ') break;
            curr_file_name[i] = tolower(file_p[j]);
            i++;
        }

        if(strcmp(curr_file_name, file_name) == 0) {
            return file_p[26] + (file_p[27] << 8);
        }
        file_p += 32; 
    }

    return -1;
}

// Copies file from disk to local sector by sector
void copy_file_to_local(char* file_p, char* new_p, int file_size, char* name) {
    int sector = get_start_sector(name, file_p);
    int address;
    int size_remaining = file_size;

    while(size_remaining) {
        if(size_remaining != file_size) {
            sector = get_fat_entry(file_p, sector);
        }
        address = SECTOR_SZ * (31+sector);

        for(int i=0; i < SECTOR_SZ; i++) {
            if(size_remaining == 0) break;
            new_p[file_size-size_remaining] = file_p[i+address];
            size_remaining--;
        }
    }

}

int main(int argc, char* argv[]) {
    int file;
    int read;
    char* file_p;
    char* new_p;
    struct stat disk;
    int file_size;

    if(argc < 3) {
        printf("ERROR: Include disk image and file as arguments\n");
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
    
    file_size = get_file_size(file_p, argv[2]);
    if(file_size > 0) {
        new_p = get_new_pointer(file_size, argv[2]);
        copy_file_to_local(file_p, new_p, file_size, argv[2]);
    } else {
        printf("ERROR: File not found\n");
    }

    close(file);
    return 0;
}