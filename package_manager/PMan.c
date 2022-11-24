#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include "pid_list.h"

int MAX_ARGS = 24;

// Starts new processes and stores process info in Linked List
void bg_entry(char commands[MAX_ARGS][MAX_ARG_LEN], int input_counter, struct Node** pid_list_head){
	pid_t child_pid;
	child_pid = fork();

	if(child_pid == 0){
		char *exe_args[input_counter-1];
		int i = 1;
		while(i < input_counter) {
			exe_args[i-1] = commands[i];
			i++;
		}
		exe_args[i-1] = NULL;
		
		if(execvp(exe_args[0], exe_args) < 0) {
			perror("Error on execvp");
			exit(0);
		}
	} else if(child_pid > 0) {
		// store information of the background child process in linked list
		add_pid(pid_list_head, child_pid, commands[1]);
	} else {
		perror("fork failed");
		exit(-1);
	}
}

// Prints all processes stored in linked list
void bg_list(struct Node* pid_list_head) {
	print_pids(pid_list_head);
}

// Kills proccess passed in by pid
void bg_kill(char pid[MAX_ARG_LEN], struct Node** pid_list_head) {
	pid_t kill_pid = atoi(pid);

	if(kill_pid == 0) {
		printf("ERROR: invalid PID\n");
	} else {
		if(kill(kill_pid, SIGKILL) == 0) {
			delete_pid(pid_list_head, kill_pid);
		} else {
			printf("ERROR: invalid PID\n");
		}
	}
}

// Pauses process passed in by pid
void bg_stop(char pid[MAX_ARG_LEN]) {
	pid_t puase_pid = atoi(pid);

	if(puase_pid == 0) {
		printf("ERROR: invalid PID\n");
	} else {
		if(kill(puase_pid, SIGSTOP) == -1) {
			printf("ERROR: invalid PID\n");
		}
	}
}

// Resumes process passed in by pid
void bg_start(char pid[MAX_ARG_LEN]) {
	pid_t start_pid = atoi(pid);

	if(start_pid == 0) {
		printf("ERROR: invalid PID\n");
	} else {
		if(kill(start_pid, SIGCONT) == -1) {
			printf("ERROR: invalid PID\n");
		}
	}
}

// Prints out process stats found in /proc/PID/
void pstat(struct Node** pid_list_head, char pid[MAX_ARG_LEN]) {
	pid_t pid_stat = atoi(pid);
	char *token;
	char stats_path[MAX_ARG_LEN];
	char status_path[MAX_ARG_LEN];
	char stats_values[64][48];
	char status_values[128][48];

	if (pid_stat == 0 || check_pid(pid_list_head, pid_stat)) {
		printf("ERROR: invalid PID\n");
		return;
	}
	sprintf(stats_path, "/proc/%d/stat", pid_stat);
	sprintf(status_path, "/proc/%d/status", pid_stat);
	
	FILE *stats_file = fopen(stats_path, "r");
	if(stats_file != NULL) {
			char stats_line[64];
			int i = 0;
			while(fgets(stats_line, 64, stats_file) != NULL) {
				token = strtok(stats_line, " ");
				while (token != NULL){
					strcpy(stats_values[i], token);
					token = strtok(NULL, " ");
					i++;
				}
			}
			fclose(stats_file);
		} else {
			printf("ERROR: could not read stats file\n");
			return;
		}
	
	FILE *status_file = fopen(status_path, "r");
	if(status_file != NULL) {
		int i = 0;
		while(fgets(status_values[i], 48, status_file) != NULL) {
			//printf("[%d]%s",i , status_values[i]);
			i++;
		}
		fclose(stats_file);
	} else {
		printf("ERROR: could not read status file\n");
		return;
	}

	printf("comm: %s\n", stats_values[1]); //get from /proc/pid/stat[1]
	printf("state: %s\n", stats_values[2]); //get from /proc/pid/stat[2]
	printf("utime: %f\n", atof(stats_values[13]) / (float)sysconf(_SC_CLK_TCK)); //get from /proc/pid/stat[13]
	printf("stime: %f\n", atof(stats_values[14]) / (float)sysconf(_SC_CLK_TCK)); //get from /proc/pid/stat[14]
	printf("rss: %s\n", stats_values[24]); //get from /proc/pid/stat[24]
	printf("%s", status_values[59]); //get from /proc/pid/status[39]
	printf("%s", status_values[60]); //get from /proc/pid/status[40]
}

void kill_all(struct Node* pid_list_head) {
	while(pid_list_head != NULL) {
		char pid_name[MAX_ARG_LEN];
		int curr_pid = pid_list_head->pid;
		sprintf(pid_name," %d", curr_pid);
		bg_kill(pid_name, &pid_list_head);
	}
}

// Checks if any of the child processes are dead and removes them from link list
void check_zombie_process(struct Node** pid_list_head) {
	int status;
	pid_t zombie_pid = 0;
	struct Node *temp = *pid_list_head;
	usleep(20000);
	
	while(temp != NULL){
		zombie_pid = waitpid(temp->pid, &status, WNOHANG);
		if(zombie_pid > 0) {
			//remove the background process from linked list
			delete_pid(pid_list_head, zombie_pid);
		} else if(zombie_pid < 0){
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
		temp = temp->next;
	}

	return;
}

int main(){
	struct Node* pid_list_head = NULL;

	while(1){
		char *command = NULL;
		size_t STR_LEN = MAX_ARG_LEN;
		command = (char *)malloc(STR_LEN);
		char *token;
		char token_arr[MAX_ARGS][MAX_ARG_LEN];
		int input_counter = 0;

		printf("PMan: >");
		getline(&command, &STR_LEN, stdin);

		// parsing input command
		token = strtok(command, " ");
		while (token != NULL && input_counter < MAX_ARGS){
			strcpy(token_arr[input_counter], token);
			token = strtok(NULL, " ");
			char *ptr = strchr(token_arr[input_counter], '\n');
			if (ptr) {
				*ptr = '\0';
			}
			input_counter ++;
		}

		// checking command and calling command function
		if (input_counter >= MAX_ARGS) {
            printf("ERROR: too many arguments entered\n");
        } else if (strcmp(token_arr[0], "bg") == 0) {
            bg_entry(token_arr, input_counter, &pid_list_head);
        } else if (strcmp(token_arr[0], "bglist") == 10 || strcmp(token_arr[0], "bglist") == 0) {
            check_zombie_process(&pid_list_head);
			bg_list(pid_list_head);
        } else if (strcmp(token_arr[0], "bgkill") == 0) {
            bg_kill(token_arr[1], &pid_list_head);
        } else if (strcmp(token_arr[0], "bgstop") == 0) {
			bg_stop(token_arr[1]);
		} else if (strcmp(token_arr[0], "bgstart") == 0) {
            bg_start(token_arr[1]);
        } else if (strcmp(token_arr[0], "pstat") == 0) {
            pstat(&pid_list_head, token_arr[1]);
        } else if (strcmp(token_arr[0], "exit") == 10 || strcmp(token_arr[0], "exit") == 0) {
			check_zombie_process(&pid_list_head);
			kill_all(pid_list_head);
            break;
        } else {
            printf("ERROR: Invalid function name: %s\n", token_arr[0]);
        }
		check_zombie_process(&pid_list_head);
	}
	return 0;
}