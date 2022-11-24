#include <stdio.h>

struct Clerk {
    int clerk_id;
    int idle;
};

void initiate_clerks(struct Clerk clerks[5]) {
    for(int i = 0; i < 5; i++){
        clerks[i].clerk_id = i+1;
        clerks[i].idle = 1;
    }
}

int idle_clerk(struct Clerk clerks[5]){
    for(int i = 0; i < 5; i++){
        if (clerks[i].idle == 1) {
            return clerks[i].clerk_id;
        }
    }

    return 0;
}