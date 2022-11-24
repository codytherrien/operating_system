#include <stdio.h>

struct Customer {
    int cid;
    int clerk_id;
    int priority;
    float entry_time;
    float process_time;
    struct Customer* next;
};

void add_cust(struct Customer** head_ref, struct Customer *new_cust) {
    if(*head_ref == NULL) {
        *head_ref = new_cust;
    } else {
        struct  Customer *curr_cust = *head_ref;
        struct Customer *temp = *head_ref;
        while(curr_cust->entry_time < new_cust->entry_time && curr_cust->next != NULL) {
            temp = curr_cust;
            curr_cust = curr_cust->next;
        }
        if(curr_cust->entry_time >= new_cust->entry_time && temp != curr_cust) {
            temp->next = new_cust;
            new_cust->next = curr_cust;
        } else {
            curr_cust->next = new_cust;
        }
    }
}

struct Customer* pop_cust(struct Customer **head_ref) {
    struct Customer *first_cust = *head_ref;
    *head_ref = first_cust->next;
    first_cust->next = NULL;
    
    return first_cust;
}