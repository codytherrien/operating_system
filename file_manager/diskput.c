/* 
Cody Therrien
V00747104
CSC 360
Assignment 3
Disk Put
*/

#include <stdio.h>
#include <time.h>
#include "Folder.h"
#include "disk_functs.h"

// Updates file inforamtion on disk
void update_dir(char* file_p, char* file_name, int file_size, int fat_idx, time_t last_mod) {
    int i;
    struct tm *ts;
    ts = localtime(&last_mod);
    int year = ts->tm_year + 1900;
	int month = (ts->tm_mon + 1);
	int day = ts->tm_mday;
	int hour = ts->tm_hour;
	int minute = ts->tm_min;

    while(file_p[0]) file_p+= 32;
    
    for(i=0; i<8; i++) {
        
        if(file_name[i] == '.') {
            file_p[i] = ' ';
            break;
        }
        file_p[i] = toupper(file_name[i]);
    }

    for(int j=0; j<3; j++) {
        file_p[j+8] = file_name[i+j+1];
    }

    
    file_p[11] = 0;
    file_p[14] = 0;
    file_p[15] = 0;
    file_p[16] = 0;
    file_p[17] = 0;

    file_p[17] |= (year - 1980) << 1;
    file_p[17] |= month >> 3;
    file_p[16] |= (month - ((file_p[17] & 0b00000001) << 3)) << 5;
    file_p[16] |= day & 0b00011111;
    file_p[15] |= (hour << 3) & 0b11111000;
    file_p[15] |= minute >> 3;
    file_p[14] |= (minute - ((file_p[15] & 0b00000111) << 3)) << 5;

    file_p[26] = fat_idx & 0xFF;
    file_p[27] = (fat_idx - file_p[26]) >> 8;
    file_p[28] = file_size & 0x000000FF;
	file_p[29] = (file_size & 0x0000FF00) >> 8;
	file_p[30] = (file_size & 0x00FF0000) >> 16;
	file_p[31] = (file_size & 0xFF000000) >> 24; 
}

// Gets file name from file path
int get_file_name(char* input, char** file_name) {
    int num_dir = 0;
    if(strchr(input, '/') == NULL) {
        *file_name = input;
        return num_dir;
    }
    char* token = strtok(input, "/");
    while(token != NULL) {
        *file_name = token;
        token = strtok(NULL, "/");
        num_dir ++;
    }
    return num_dir;
}

// Checks if file is already present in directory 
int file_not_in_folder(char* file_p, char* file_name) {
    char* curr_name = malloc(sizeof(char));
    int i;
    char file_type;

    while(file_p[0]) {
        file_type = 'F';
        for(i=0; i<8; i++) {
            if(file_p[i] == ' ') break;
            curr_name[i] = tolower(file_p[i]);
        }
        if((file_p[11] & 0b00010000) == 0b00010000) {
           file_type = 'D';
        }
        if(file_type == 'F') {
            curr_name[i] = '.';
            i++;
            for(int j=8; j<11; j++) {
                if(file_p[j] == ' ') break;
                curr_name[i] = tolower(file_p[j]);
                i++;
            }
        }

        if(strcmp(file_name, curr_name) == 0) {
            return 0;
        }
        file_p += 32;
    }
    return 1;
}

// Updates FAT entry
void set_FAT(char* fat_p, int idx, int value) {
    if(idx % 2 == 0) {
        idx = 3*idx / 2;
        fat_p[SECTOR_SZ + idx + 1] = (value >> 8) & 0x0F;
        fat_p[SECTOR_SZ + idx] = value & 0xFF;
    } else {
        idx = 3*idx / 2;
        fat_p[SECTOR_SZ + idx + 1] = (value >> 4) & 0xFF;
        fat_p[SECTOR_SZ + idx] = (value << 4) & 0xF0;
    }
}

// Copies file inforamtion from local to disk
void copy_file(char* fat_p, char* file_p, char* new_p, char* file_name, int file_size, time_t last_mod) {
    int curr_idx = 2;
    int next_idx;
    int sz_left = file_size;
    int curr_address;

    while(get_fat_entry(fat_p, curr_idx)) curr_idx++;

    update_dir(file_p, file_name, file_size, curr_idx, last_mod);
    while(sz_left > 0) {
        curr_address = (curr_idx + 31) * SECTOR_SZ;
        
        for(int i=0; i<SECTOR_SZ; i++) {
            if(sz_left == 0) {
                set_FAT(fat_p, curr_idx, 0xFF);
                return;
            }
            fat_p[curr_address+i] = new_p[file_size-sz_left];
            sz_left --;
        }
        set_FAT(fat_p, curr_idx, 0x69);
        next_idx = curr_idx;
        while(get_fat_entry(fat_p, next_idx)) next_idx++;
        set_FAT(fat_p, curr_idx, next_idx);
        curr_idx = next_idx; 
    }

}

// Finds correct directory to store file in
void transfer_file(char* file_p, char* new_p, char* input, int file_size, char* file_name, time_t last_mod) {
    char* og_p = file_p;
    file_p += SECTOR_SZ*19;
    char* token = strtok(input, "/");
    struct Folder* que = NULL;
    int found_folder;
    struct Folder* folder = malloc(sizeof(struct Folder));
    folder->file_p = file_p;
    folder->name = "Root";
    folder->next = NULL;

    add_folder(&que, folder);
    while(strcmp(token, file_name) != 0) {
        
        
        found_folder = 0;
        folder = pop_folder(&que);
        file_p = folder->file_p;

        while(file_p[0]) {
            char* folder_name = malloc(sizeof(char));
            for(int i=0; i<8; i++) {
                if(file_p[i] == ' ') break;
                folder_name[i] = tolower(file_p[i]);
            }
            

            if((file_p[11] & 0b00010000) == 0b00010000 && strcmp(token, folder_name) == 0) {
                folder->file_p = og_p + SECTOR_SZ*(31 + (file_p[26] + (file_p[27] << 8)));  
                folder->name = folder_name;
                folder->next = NULL;
                add_folder(&que, folder);
                found_folder = 1;
            }
            file_p += 32;
        }
        file_p = folder->file_p;
        if(!found_folder) {
            printf("The directory not found\n");
            exit(-1);
        }
        token = strtok(NULL, "/");
        
    }
    if(file_not_in_folder(file_p, file_name)) {
        copy_file(og_p, file_p, new_p, file_name, file_size, last_mod);
    } else {
        printf("ERROR: Files already exists\n");
        exit(-1);
    }
}

int main(int argc, char*argv[]) {
    int file;
    int file2;
    int read;
    char* file_p;
    char* new_p;
    struct stat disk;
    struct stat input;
    int file_size;
    int disk_size;
    int free_space;
    time_t last_mod;
    char* read_input = malloc(sizeof(char));
    char* file_name = malloc(sizeof(char));

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
        printf("ERROR: Memory mapping failed\n");
        exit(-1);
    }
    strcpy(read_input, argv[2]);
    get_file_name(argv[2], &file_name);
    file2 = open(file_name, O_RDWR);
    if(file2 < 0) {
        printf("File not found\n");
        exit(-1);
    }

    read = fstat(file2, &input);
    if(read < 0) {
        printf("ERROR: Failed to write input file buffer\n");
        exit(-1);
    }
    file_size = input.st_size;
    last_mod = input.st_mtime;
    new_p = mmap(NULL, file_size, PROT_READ, MAP_SHARED, file2, 0);
    if(new_p == MAP_FAILED) {
        printf("ERROR: Input file memory mapping failed\n");
        exit(-1);
    }

    get_size(file_p, &disk_size);
    get_free_space(file_p, &free_space, disk_size);

    if(free_space >= file_size) {
        transfer_file(file_p, new_p, read_input, file_size, file_name, last_mod);

    } else {
        printf("Not enough free space in the disk image\n");
        exit(-1);
    }
}