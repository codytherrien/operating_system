struct Folder {
    char* file_p;
    char* name;
    struct Folder* next;
};

// Push folder on priority Queue
void add_folder(struct Folder** que, struct Folder* folder) {
    if(*que == NULL) {
        *que = folder;
    } else {
        struct Folder* temp = *que;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = folder;
    }
}

// Pop folder off priority Queue
struct Folder* pop_folder(struct Folder** que) {
    struct Folder* new_folder = *que;
    *que = new_folder->next;
    new_folder->next = NULL;

    return new_folder;
}