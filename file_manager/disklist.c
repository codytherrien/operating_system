/* 
Cody Therrien
V00747104
CSC 360
Assignment 3
Disk List
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "Folder.h"
#include "disk_functs.h"

// Returns the month string from int.
// If invalid month is passed in sets month to Jan
char* get_month(int month) {
    char* month_str;
    if(month == 1) month_str = "Jan";
    else if(month == 2) month_str = "Feb";
    else if(month == 3) month_str = "March";
    else if(month == 4) month_str = "April";
    else if(month == 5) month_str = "May";
    else if(month == 6) month_str = "June"; 
    else if(month == 7) month_str = "July";
    else if(month == 8) month_str = "Aug";
    else if(month == 9) month_str = "Sept";
    else if(month == 10) month_str = "Oct";
    else if(month == 11) month_str = "Nov";
    else if(month == 12) month_str = "Nov";
    else month_str = "Jan";

    return month_str;
}

//Prints the files on the disk
void list_dir(char* file_p) {
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
        if(strcmp(folder->name, "Root") != 0) {
            printf("/");
        }
        printf("%s\n\n", folder->name);
        printf("==============\n\n");


        while(file_p[0]) {
            char file_type = 'F';
            char* file_name = malloc(sizeof(char));
            char* folder_name = malloc(sizeof(char));
            unsigned int file_size;
            int i;

            for(i=0; i<8; i++) {
                if(file_p[i] == ' ') break;
                file_name[i] = tolower(file_p[i]);
            }

            if((file_p[11] & 0b00010000) == 0b00010000 && file_name[0] != '.') {
                file_type = 'D';
                folder->file_p = og_p + SECTOR_SZ*(31 + (file_p[26] + (file_p[27] << 8)));  
                folder_name = folder->name;
                if(strcmp(folder_name, "Root") != 0) {
                    strcat(folder_name, "/");
                    strcat(folder_name, file_name);
                    folder->name = folder_name;
                } else {
                    folder->name = file_name;
                }
                folder->next = NULL;
                add_folder(&que, folder); 
            }

            if(file_type == 'F') {
                file_name[i] = '.';
                i++;
                for(int j=8; j<11; j++) {
                    if(file_p[j] == ' ') break;
                    file_name[i] = tolower(file_p[j]);
                    i++;
                }
            }

            if ((file_p[11] & 0b00000010) == 0 && (file_p[11] & 0b00001000) == 0 && file_name[0] != '.') {
                file_size = file_p[28] + (file_p[29] << 8) + (file_p[30] << 16) + (file_p[31] << 24);
                int minute;
                int hour;
                int day;
                int month;
                int year;

                if (file_type == 'F') {
                    minute = ((file_p[15] & 0b00000111) << 3) + ((file_p[14] & 0b11100000) >> 5);
                    hour = (file_p[15] & 0b11111000) >> 3;
                    day = (file_p[16] & 0b00011111);
                    month = ((file_p[17] & 0b00000001) << 3) + ((file_p[16] & 0b11100000) >> 5);
                    year = 1980 + ((file_p[17] & 0b11111110) >> 1);
                } else {
                    minute = ((file_p[23] & 0b00000111) << 3) + ((file_p[22] & 0b11100000) >> 5);
                    hour = (file_p[23] & 0b11111000) >> 3;
                    day = (file_p[24] & 0b00011111);
                    month = ((file_p[25] & 0b00000001) << 3) + ((file_p[24] & 0b11100000) >> 5);
                    year = 1980 + ((file_p[25] & 0b11111110) >> 1);
                }

                char* month_str = get_month(month);

                printf("%c %10d %20s %s %d %d %02d:%02d\n", file_type, file_size, file_name, month_str, day, year, hour, minute);
            }
            file_p += 32; 
        }
        printf("\n");
    } 
}


int main(int argc, char* argv[]) {
    int file;
    int read;
    char* file_p;
    struct stat disk;

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

    list_dir(file_p);
}