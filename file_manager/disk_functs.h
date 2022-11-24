#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
int SECTOR_SZ = 512;

// Returns fat entry for sector passed in as parameter
int get_fat_entry(char* file_p, int sector) {
    int first;
    int second;

    
    if(sector % 2) {
        sector = (3*sector) /2;
        first = file_p[SECTOR_SZ + sector] & 0xF0;
        second = file_p[SECTOR_SZ + sector + 1] & 0xFF;
        return (first >> 4) + (second << 4);
    }
    sector = (3*sector) /2;
    first = file_p[SECTOR_SZ + sector + 1] & 0x0F;
    second = file_p[SECTOR_SZ + sector] & 0xFF;
    return (first << 8) + second;
}

// Updates disk size
void get_size(char* file_p, int* disk_size){
    *disk_size = file_p[11] + (file_p[12] << 8) * (file_p[19] + (file_p[20] << 8));
}

// Updates free space
void get_free_space(char* file_p, int* free_space, int disk_size){
    for(int i=2; i<disk_size/SECTOR_SZ; i++){
    //for(int i=2; i<9; i++){
        if(!get_fat_entry(file_p, i)) *free_space += 1;
    }
    *free_space = (*free_space-31) * SECTOR_SZ;
}