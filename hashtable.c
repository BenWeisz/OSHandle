#include <stdlib.h>
#include <stdio.h>

#define T int*
#define MAX_BINS 1000

// Hash Entry for File Objects
typedef struct HashEntry {
    T value;
    int key;
    struct HashEntry* next; 
} HashEntry;

// Hash Table
typedef HashEntry** HashTable;

// Allocate enough space and initialize a new hash entry
HashEntry* hash_entry_init(int key, T value) {
    HashEntry* new_hash_entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (new_hash_entry == NULL) {
        perror("ERROR: Could not allocate data for new hash entry\n");
        return NULL;
    }

    new_hash_entry->key = key;
    new_hash_entry->value = value;
    new_hash_entry->next = NULL;
}

// Deallocate the space provided for the given hash entry and return its contents for possible deallocation
void hash_entry_destroy(HashEntry** hash_entry_ptr, T* data) {
    HashEntry* hash_entry = *hash_entry_ptr;
    *data = hash_entry->value;

    free(hash_entry);
    *hash_entry_ptr = NULL;
}

// Allocate enough space for the hashtable
HashTable hash_table_init() {
    HashTable hash_table = (HashTable)malloc(sizeof(HashEntry*) * MAX_BINS);
    if (hash_table == NULL) {
        perror("ERROR: Could not allocate data for new hash table\n");
        return NULL;
    }

    return hash_table;
}

// Deallocate all the space used for the hashtable
void hash_table_destroy(HashTable** hash_table) {

}

// Hash function for the hash table
int hash_table_hash_function(int key) {
    return key % MAX_BINS;
}

int main() {
    return 0;
}