#include <stdlib.h>
#include <stdio.h>

// FreeList data queue for reusing inactive fd's
typedef struct FreeListNode {
    int fd;
    struct FreeListNode* next; 
} FreeListNode;

typedef struct FreeList {
    struct FreeListNode* front;
    struct FreeListNode* back;
    int length;
} FreeList;

// Allocate space for the free list structure
FreeList* free_list_init() {
    FreeList* new_free_list = (FreeList*)malloc(sizeof(FreeList));
    if (new_free_list == NULL) {
        perror("ERROR: Could not allocate data for free list\n");
        return NULL;
    }

    new_free_list->front = NULL;
    new_free_list->back = NULL;
    new_free_list->length = 0;

    return new_free_list;
}

// Deallocate all of the free list's memory
void free_list_destroy(FreeList** free_list_ptr) {
    FreeList* free_list = *free_list_ptr;
    
    FreeListNode* curr_node = free_list->front;
    FreeListNode* next_node;
    // Free all the elements in the free list
    while (curr_node != NULL) {
        next_node = curr_node->next;
        free(curr_node);

        curr_node = next_node;
    }

    free(free_list);
    *free_list_ptr = NULL;
}

// Peak the value of the first element in the list
int free_list_peak(FreeList* free_list) {
    // Ensure that the free list isn't empty
    if (free_list->length == 0) {
        fprintf(stderr, "ERROR: Cannot peak first value in list when it is empty\n");
        return -1;
    }

    return free_list->front->fd;
}

// Pop the first element of the list
void free_list_pop(FreeList* free_list) {
    // Ensure that the free list isn't empty
    if (free_list->length == 0) {
        fprintf(stderr, "ERROR: Cannot pop element when list is empty\n");
        return;
    }

    // Remove the front node
    FreeListNode* front_node = free_list->front;
    if (front_node->next == NULL)
        free_list->back = NULL;

    free_list->front = front_node->next;

    free(front_node);

    free_list->length--;
}

void free_list_push(FreeList* free_list, int fd){
    FreeListNode* new_node = (FreeListNode*)malloc(sizeof(FreeListNode));
    if (new_node == NULL) {
        perror("ERROR: Could not allocate data for new FreeList element\n");
        return;
    }

    new_node->fd = fd;
    new_node->next = NULL;

    // Add in the new free list node
    if (free_list->front == NULL) {
        free_list->front = new_node;
        free_list->back = new_node;
    } else {
        free_list->back->next = new_node;
        free_list->back = new_node;
    }

    free_list->length++;
}


int main() {
    FreeList* free_list = free_list_init();
    free_list_push(free_list, 1);
    free_list_push(free_list, 2);
    free_list_push(free_list, 3);
    free_list_push(free_list, 4);
    free_list_push(free_list, 5);
    free_list_push(free_list, 6);

    free_list_destroy(&free_list);

    return 0;
}