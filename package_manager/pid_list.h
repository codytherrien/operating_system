#include <stdio.h>
#include <stdlib.h>

int MAX_ARG_LEN = 64;

struct Node {
    pid_t pid;
    char pid_name[64];
    struct Node* next;
};

// Adds process node to Linked List
void add_pid(struct Node** head_ref, int _pid, char _name[MAX_ARG_LEN]) {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    char _pid_name[MAX_ARG_LEN*2];

    sprintf(_pid_name, "%d", _pid);
    strcat(_pid_name, ":  ");
    strcat(_pid_name, _name);
    new_node->pid = _pid;
    strcpy(new_node->pid_name, _pid_name);
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
    } else {
        struct Node *curr_node = *head_ref;
        while(curr_node->next != NULL) {
            curr_node = curr_node->next;
        }
        curr_node->next = new_node;
    }

}

// Removes process node from linked list
void delete_pid(struct Node** head_ref, int _pid) {
    struct Node *temp = *head_ref, *prev;

    if (temp != NULL && temp->pid == _pid) {
        *head_ref = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && temp->pid != _pid) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("ERROR: pid not found\n");
        return;
    }

    prev->next = temp->next;
    free(temp);
}

// Prints all nodes in the linked list
void print_pids(struct Node* n) {
    int pid_count = 0;
    while (n != NULL) {
        printf("%s\n", n->pid_name);
        n = n->next;
        pid_count ++;
    }
    printf("Total background jobs: %d\n", pid_count);
}

// Checks if a given node is in the Linked List
int check_pid(struct Node** head_ref, int _pid) {
    struct Node *temp = *head_ref;

    while(temp != NULL) {
        if(temp->pid == _pid) {
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}