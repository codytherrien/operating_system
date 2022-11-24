#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "Clerks.h"
#include "Customers.h"

// Global Variables
struct Clerk clerks[5];
struct Customer *economy_q = NULL;
struct Customer *priority_q = NULL;
struct timeval start;
int total_custs[2] = {0, 0};
int queue_lens[2] = {0, 0};
float wait_times[2] = {0, 0};
pthread_cond_t arrival_cond;
pthread_cond_t clerk_cond;
pthread_mutex_t lock;

void invalid_input() {
    printf("ERROR: Invalid Input\n");
    exit(-1);
}

// Fuction Simulating customer entering queue
void enter_queue(struct Customer* cust) {
    if(cust->priority) {
        total_custs[0] ++;
        queue_lens[0] ++;
        printf("A customer enters a queue: the queue ID 1,");
        printf(" and the length of the queue %2d. \n", queue_lens[0]);
        add_cust(&priority_q, cust);
    } else {
        total_custs[1] ++;
        queue_lens[1] ++;
        printf("A customer enters a queue: the queue ID 0,");
        printf(" and the length of the queue %2d. \n", queue_lens[1]);
        add_cust(&economy_q, cust);
    }
}

// Function Simulating customer processed by clerk
void enter_clerk(struct Customer* cust) {
    struct timeval stop;
    double time;

    gettimeofday(&stop, NULL);
    time = ((double)(stop.tv_sec - start.tv_sec) * 1000000 + (double)(stop.tv_usec - start.tv_usec)) / 100000;
    if(cust->priority) {
        wait_times[0] += time - cust->entry_time;
    } else {
        wait_times[1] += time - cust->entry_time;
    }
    printf("A clerk starts serving a customer: start time %.2f", time/10);
    printf(" the customer ID %2d, the clerk ID %1d. \n", cust->cid, cust->clerk_id);
    
    usleep((int)cust->process_time*100000);

    gettimeofday(&stop, NULL);
    time = ((double)(stop.tv_sec - start.tv_sec) * 1000000 + (double)(stop.tv_usec - start.tv_usec)) / 100000;
    printf("-->>> A clerk finished serving a customer: end time %.2f, ", time/10);
    printf("the customer ID %2d, the clerk ID %1d. \n", cust->cid, cust->clerk_id);
}

// Customer Thread Function
void* cust_thread(void * arg) {
    struct Customer* cust = (struct Customer*) (arg);

    usleep((int)cust->entry_time*100000);
    printf("A customer arrives: customer ID %2d. \n", cust->cid);

    //Start of Critical Section
    pthread_mutex_lock(&lock);
    enter_queue(cust);
    pthread_cond_signal(&arrival_cond);
    while(cust->clerk_id == 0) {
        pthread_cond_wait(&clerk_cond, &lock);
    }

    if(cust->priority) {
        pop_cust(&priority_q);
        queue_lens[0] --;
    } else {
        pop_cust(&economy_q);
        queue_lens[1] --;
    }

    clerks[cust->clerk_id-1].idle = 0;
    pthread_mutex_unlock(&lock); // End of Critical Section

    enter_clerk(cust);

    //Enter critical Section
    pthread_mutex_lock(&lock);
    clerks[cust->clerk_id-1].idle = 1;
    pthread_mutex_unlock(&lock); // End of Critical Section
    pthread_cond_signal(&arrival_cond);
    
    pthread_exit(NULL);
}

// Managing the customer queues (main thread)
void queue_management(int num_custs) {
    int clerk_id;

    while(queue_lens[0] > 0 || queue_lens[1] > 0 || total_custs[0] + total_custs[1] < num_custs) {
        pthread_mutex_lock(&lock);
        clerk_id = idle_clerk(clerks);
        while(clerk_id == 0) {
            pthread_cond_wait(&arrival_cond, &lock);
            clerk_id = idle_clerk(clerks);
        }
        
        if(queue_lens[0] > 0) {
            priority_q->clerk_id = clerk_id;
        } else if (queue_lens[1] > 0) {
            economy_q->clerk_id = clerk_id;
        }
        pthread_mutex_unlock(&lock);
        pthread_cond_broadcast(&clerk_cond);
    }
}

// Comuputing wait times after all customer have left the system
void compute_wait_times() {
    float ave_wait = (wait_times[0] + wait_times[1]) / (10*(total_custs[0] + total_custs[1]));

    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", ave_wait);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", wait_times[0]/(10*(float)total_custs[0]));
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", wait_times[1]/(10*(float)total_custs[1]));
}


int main(int argc, char **argv) {
    FILE *fp;
    char *line;
    size_t len = 0;
    ssize_t read;
    char* token;
    int num_custs;
    struct Customer new_cust;
    int err = 0;

    initiate_clerks(clerks);

    if(argc < 2) {
        printf("ERROR: No file name given\n");
        exit(-1);
    }

    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        exit(-1);
    }

    read = getline(&line, &len, fp);
    if(read == -1) {
        printf("ERROR: Invalid Input");
        exit(-1);
    }
    sscanf(line, "%d", &num_custs);
    struct Customer custs[num_custs];
    pthread_t tid[num_custs];
    
    for(int i=0; i<num_custs; i++) {
        read = getline(&line, &len, fp);
        if(read == -1) {
            invalid_input();
        }
        token = strtok(line, ":");
        if(token == NULL) invalid_input();
        sscanf(token, "%d", &new_cust.cid);
        token = strtok(NULL, ",");
        if(token == NULL) invalid_input();
        sscanf(token, "%d", &new_cust.priority);
        token = strtok(NULL, ",");
        if(token == NULL) invalid_input();
        sscanf(token, "%f", &new_cust.entry_time);
        token = strtok(NULL, ",");
        if(token == NULL) invalid_input();
        sscanf(token, "%f", &new_cust.process_time);
        new_cust.next = NULL;
        
        custs[i] = new_cust;
    }

    fclose(fp);
    if(line) {
        free(line);
    }

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    gettimeofday(&start, NULL);
    // Start Customer Threads
    for(int i=0; i<num_custs; i++) {
        if((err = pthread_create(&tid[i], NULL, cust_thread, &custs[i]))!=0) {
            perror("Error while creating thread\n");
        }
    }
    queue_management(num_custs);

    for(int i=0; i<num_custs; i++) {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&lock);
    printf("All Customers Have Left the System \n\n");
    compute_wait_times();

    return 0; 
}
