#include <stdlib.h>
#include <stdio.h>

#define T int

typedef struct ListNode {
    struct ListNode* next;
    T value;
} ListNode;

typedef struct List {
    struct ListNode* front;
    struct ListNode* back;
    int length;
} List;

// Forward Declarations
void list_node_init(ListNode** list_node, T value);
void list_init(List** list);
void list_destroy(List* list);
void list_remove(List* list, int index, T* value);
void list_push_back(List* list, T value);
void list_pop_front(List* list, T* value);
T list_find(List* list, int (*lambda)(T), T* value);
T list_front(List* list);

/* Initialize a new ListNode */
void list_node_init(ListNode** list_node, T value) {
    ListNode* new_list_node = (ListNode*)malloc(sizeof(ListNode));
    new_list_node->value = value;
    *list_node = new_list_node;
}

/* Initialize the structure for a new list */
void list_init(List** list) {
    List* new_list = (List*)malloc(sizeof(List));
    new_list->front = NULL;
    new_list->back = NULL;
    new_list->length = 0;

    *list = new_list;
}

/* Delete the given List data structure */
void list_destroy(List* list) {
    T value;
    while (list->length > 0)
        list_pop_front(list, &value);

    free(list);
}

/* Remove the element at index `index` and return its contents for possible deallocation */
void list_remove(List* list, int index, T* value) {
    // Error check that use isn't removing a bad index
    if (index < 0 || index >= list->length) {
        printf("ERROR: Removing bad index\n");
        return;
    }

    // If trying to remove the first ListNode, pop the first node
    if (index == 0) {
        list_pop_front(list, value);
        return;
    }

    // Find the ListNode to remove
    ListNode* curr_node = list->front;
    ListNode* prev_node = NULL;
    int i = 0;
    while (i != index) {
        prev_node = curr_node;
        curr_node = curr_node->next;
        i++;
    }

    prev_node->next = curr_node->next;
    
    // Ensure that the last pointer is updated if we remove the last node
    if (prev_node->next == NULL)
        list->back = prev_node;

    // Return the node value for possible deallocation and deallocate the ListNode
    *value = curr_node->value;
    free(curr_node);

    // Decrease the list length
    list->length--;
}

/* Push a new element to the back of the list */
void list_push_back(List* list, T value) {
    // Create the new ListNode and set its value
    ListNode* new_list_node;
    list_node_init(&new_list_node, value);

    // If there are already some nodes in the list
    if (list->back != NULL) {
        list->back->next = new_list_node;
        list->back = new_list_node;
    } else {
        list->front = new_list_node;
        list->back = new_list_node;
    }

    list->length++;
}

/* Pop the first element from the list */
void list_pop_front(List* list, T* value) {
    // Error check to make sure list isn't empty
    if (list->front == NULL) {
        printf("ERROR: Cannot pop the front of an empty list\n");
        return;
    }

    ListNode* front_node = list->front;

    // Whether next node exists or not, set the front to the next node
    list->front = front_node->next;

    // If the next node doesn't exist set the back to NULL
    if (front_node->next == NULL)
        list->back = NULL;

    list->length -= 1;

    // Deallocate this ListNode and return the contents for possible deallocation
    *value = front_node->value;
    free(front_node);
}

/* Find the first ListNode who's contents result in a truthy value when called on the function lambda */
int list_find(List* list, int (*lambda)(T), T* value) {
    ListNode* curr_node = list->front;

    while (curr_node != NULL) {
        if ((*lambda)(curr_node->value)) {
            *value = curr_node->value;
            return 1;
        }
        curr_node = curr_node->next;
    }

    return 0;
}

/* Return the contents of the ListNode at the front of the list */
T list_front(List* list) {
    return list->front->value;
}

int find_neg(T a) {
    if(a < 0)
        return 1;
    return 0;
}

int main() {
    List* list;
    list_init(&list);

    list_push_back(list, 10);
    list_push_back(list, -10);
    list_push_back(list, 7);

    int a;
    list_remove(list, 2, &a);
    list_remove(list, 1, &a);

    int b;
    int r = list_find(list, find_neg, &b);
    printf("%d\n", r);

    list_destroy(list);

    return 0;
}