#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// The maximum hash bins for the IOFileHashTable to use
#define MAX_HASHTABLE_BINS 2

///////////////////////////////////////
/* Simple file system in a directory */
///////////////////////////////////////

// File system file model
typedef struct FSFile {
    char* filename;
    char* data;
    int size;
    struct FSFile* next;
} FSFile;

// File system container
typedef struct FileSystem {
    struct FSFile* front;
    struct FSFile* back;
} FileSystem;

FileSystem* fs_module = NULL;

// FUNCTIONS FOR FSFile
// Initialize the FSFile
FSFile* file_system_file_init(const char* filename) {
    FSFile* new_file = (FSFile*)malloc(sizeof(FSFile));
    if (new_file == NULL) {
        perror("ERROR: Could not allocate data for FSFile\n");
        return NULL;
    }
    
    // Initialize file name
    size_t filename_length = strlen(filename);
    new_file->filename = (char*)malloc(sizeof(char) * (filename_length + 1));
    if (new_file == NULL) {
        perror("ERROR: Could not allocate data for FSFile filename\n");
        return NULL;
    }

    strcpy(new_file->filename, filename);
    
    // Initialize the data pointer
    new_file->data = NULL;
    new_file->size = 0;

    // Initialize the next FSfile pointer
    new_file->next = NULL;

    return new_file;
}

// Deallocate FSFile
void file_system_file_destroy(FSFile** file_ptr) {
    FSFile* file = *file_ptr;
    free(file->filename);
    free(file->data);
    free(file);

    *file_ptr = NULL;
}

// Set the data for the given FSFile
void file_system_file_set_data(FSFile* file, const char* data, size_t size) {
    if (file->data != NULL) {
        fprintf(stderr, "ERROR: You cannot set the file data twice\n");
        return;
    }

    // Allocate enough space for data
    file->data = (char*)malloc(sizeof(char) * size);
    if (file->data == NULL) {
        perror("ERROR: Could not allocate space for FSFile data\n");
        return;
    }
    
    file->size = size;

    // Copy the data into the buffer
    strncpy(file->data, data, size);
}

// FUNCTIONS FOR FileSystem
// Initialize the FileSystem
FileSystem* file_system_init() {
    FileSystem* file_system = (FileSystem*)malloc(sizeof(FileSystem));
    if (file_system == NULL) {
        perror("ERROR: Could not allocate data for FileSystem\n");
        return NULL;
    }

    file_system->front = NULL;
    file_system->back = NULL;

    return file_system;
}

// Destroy the FileSystem
void file_system_destroy(FileSystem** file_system_ptr) {
    FileSystem* file_system = *file_system_ptr;
    
    FSFile* curr_file = file_system->front;
    FSFile* next_file;
    while (curr_file != NULL) {
        next_file = curr_file->next;
        file_system_file_destroy(&curr_file);

        curr_file = next_file;
    }

    free(file_system);
    *file_system_ptr = NULL;
}

int file_system_add_file(FileSystem* file_system, const char* filename, const char* data, size_t size) {
    FSFile* file = file_system_file_init(filename);
    if (file == NULL) {
        fprintf(stderr, "ERROR: Failed to add a file to file system\n");
        return 0;
    }

    // Assign the data to the FSFile
    file_system_file_set_data(file, data, size);

    // Attach the file to the file system
    if (file_system->front == NULL) {
        file_system->front = file;
        file_system->back = file;
    } else {
        file_system->back->next = file;
        file_system->back = file;
    }

    return 1;
}

FSFile* file_system_find_file(FileSystem* file_system, const char* filename) {
    FSFile* curr_file = file_system->front;
    while (curr_file != NULL) {
        if (strcmp(filename, curr_file->filename) == 0)
            break;

        curr_file = curr_file->next;
    }

    if (curr_file == NULL) {
        errno = ENOENT;
        return NULL;
    }

    return curr_file;
}

// FUNCTIONS TO SET UP A BASIC ENVIRONMENT
// Set up a basic environment
int fs_environment_init() {
    fs_module = file_system_init();
    if (fs_module == NULL) {
        return 0;
    }

    const char* filename1 = "file1.txt";
    const char* data1 = "Test Data1";
    int created = file_system_add_file(fs_module, filename1, data1, 10);
    if (!created) {
        file_system_destroy(&fs_module);
        return 0;
    }

    const char* filename2 = "file2.txt";
    const char* data2 = "Test Data2";
    created = file_system_add_file(fs_module, filename2, data2, 10);
    if (!created) {
        file_system_destroy(&fs_module);
        return 0;
    }

    return 1;
}

// Tear down the file system environment
void fs_environment_destroy() {
    file_system_destroy(&fs_module);
}

///////////////////////////////////////////
/* Free list structures for reusing fd's */
///////////////////////////////////////////

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

// Push an fd to the back of the list
int free_list_push(FreeList* free_list, int fd){
    FreeListNode* new_node = (FreeListNode*)malloc(sizeof(FreeListNode));
    if (new_node == NULL) {
        // errno is set by malloc
        return 0;
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

    return 1;
}

//////////////////////////////////////////////////////////
/* Hash table for rapid access of file objects (IOFile) */
//////////////////////////////////////////////////////////

// This file will possibly be linked to another IOFile who's fd hash is the same as this file's fd hash
typedef struct IOFile {
    int fd;
    int cursor_pos;
    unsigned int mode_type;
    struct FSFile* fs_file;
    struct IOFile* next;
} IOFile;

#define IOFILE_MODE_READ 0x01
#define IOFILE_MODE_WRITE 0x02

// Hash table will consist of array of IOFile pointers
typedef IOFile** IOFileHashTable;

// Allocate enough space to store all of the pointer to the IOFile buckets
IOFileHashTable io_file_hash_table_init() {
    IOFileHashTable new_hash_table = (IOFileHashTable)malloc(sizeof(IOFile*) * MAX_HASHTABLE_BINS);
    if (new_hash_table == NULL) {
        perror("ERROR: Could not allocate data for new IOFileHashTable\n");
        return NULL;
    }

    // Initialize all the buckets to contain no IOFiles
    for (int i = 0; i < MAX_HASHTABLE_BINS; i++)
        new_hash_table[i] = NULL;

    return new_hash_table;
}

// Deallocate any memory used for the IOFileHashTable structure
void io_file_hash_table_destroy(IOFileHashTable* hash_table_ptr) {
    IOFileHashTable hash_table = *hash_table_ptr;

    for (int i = 0; i < MAX_HASHTABLE_BINS; i++) {
        // Only clean up the bucket if it was at some point in use
        IOFile* curr_file = hash_table[i];
        if (curr_file != NULL) {
            IOFile* next_file;
            
            // Clean up all the IOFile structures in this bucket
            while (curr_file != NULL) {
                next_file = curr_file->next;
                free(curr_file);

                curr_file = next_file;
            }
        }
    }

    // Free the data allocated for the whole hash table
    free(hash_table);
    *hash_table_ptr = NULL;
}

// The hash function for choosing a bucket for the IOFile in the hashtable
int io_file_hash_table_hash(int fd) {
    return fd % MAX_HASHTABLE_BINS;
}

// Add a new IOFile to the hashtable with its assigned fd and FSFile pointer
void io_file_hash_table_new_file(IOFileHashTable hash_table, int fd, unsigned int mode_type, FSFile* fs_file) {
    IOFile* new_io_file = (IOFile*)malloc(sizeof(IOFile));
    if (new_io_file == NULL) {
        // malloc will set ENOMEM
        return;
    }

    // Initialize the new IOFile
    new_io_file->fd = fd;
    new_io_file->cursor_pos = 0;
    new_io_file->mode_type = mode_type;
    new_io_file->fs_file = fs_file;

    // Place the IOFile into its own bucket
    int hash = io_file_hash_table_hash(fd);
    new_io_file->next = hash_table[hash];
    hash_table[hash] = new_io_file;
}

// Get the IOFile pertaining to the given fd
IOFile* io_file_hash_table_get_file(IOFileHashTable hash_table, int fd) {
    // Get the bucket for the IOFile
    int hash = io_file_hash_table_hash(fd);

    // Filter for particular IOFile
    IOFile* curr_file = hash_table[hash];
    while (curr_file != NULL) {
        if (curr_file->fd == fd)
            break;
        curr_file = curr_file->next;
    }

    // Set the errno if the file descriptor was invalid
    if (curr_file == NULL)
        errno = EBADF;

    return curr_file;
}

// Deallocate the IOFile pertaining to the given fd
int io_file_hash_table_remove_file(IOFileHashTable hash_table, int fd) {
    // Get hash for the fd
    int hash = io_file_hash_table_hash(fd);

    // Maintain the previous file in the bucket to allow for re-linking of bucket elements
    IOFile* curr_file = hash_table[hash];
    IOFile* prev_file = NULL;
    while (curr_file != NULL) {
        // If the IOFile is found patch up the links in the bucket before deallocation
        if (curr_file->fd == fd) {
            if (prev_file != NULL)
                prev_file->next = curr_file->next;
            else
                hash_table[hash] = curr_file->next;
            break;
        }

        prev_file = curr_file;
        curr_file = curr_file->next;
    }

    // Ensure that the IOFile associated with the fd exists
    if (curr_file == NULL) {
        errno = EBADF;
        return 0;
    }

    // Deallocate the memory associated with the IOFile in the hashtable
    free(curr_file);

    // A successful removal
    return 1;
}

/////////////////////////
/* File Descriptor API */
/////////////////////////
typedef struct IOModule {
    IOFileHashTable hash_table;
    FreeList* free_list;
    int next_fd;
} IOModule;

IOModule* io_module = NULL;

// Initialize the IOModule
int io_module_init() {
    // Allocate space for new IOModule
    io_module = (IOModule*)malloc(sizeof(IOModule));
    if (io_module == NULL) {
        perror("ERROR: Could not allocate data for IOModule\n");
        return 0;
    }

    // Initialize the new IOModule
    io_module->hash_table = io_file_hash_table_init();
    if (io_module->hash_table == NULL) {
        free(io_module);
        return 0;
    }
    
    io_module->free_list = free_list_init();
    if (io_module->free_list == NULL) {
        io_file_hash_table_destroy(&io_module->hash_table);
        free(io_module);
        return 0;
    }

    io_module->next_fd = 0;

    return 1;
}

// Deallocate the structures associated with the IOModule
void io_module_destory() {
    // Deallocate all the data structures used by the module
    io_file_hash_table_destroy(&io_module->hash_table);
    free_list_destroy(&io_module->free_list);
    free(io_module);

    io_module = NULL;
}

int io_module_create_new_fd() {
    int selected_fd = -1;
    // If we have some previously used fds we want to resuse them
    if (io_module->free_list->length != 0) {
        selected_fd = free_list_peak(io_module->free_list);
        free_list_pop(io_module->free_list);
    }
    // Otherwise use the next highest fd
    else {
        selected_fd = io_module->next_fd;
        io_module->next_fd++;
    }

    return selected_fd;    
}

// The API call to open a new IOFile and return a new file descriptor
int io_open(const char* filename, unsigned int mode_type) {
    // Ensure user provides an io mode
    if (mode_type == 0) {
        fprintf(stderr, "ERROR: Provide at least one of the io modes: IOFILE_MODE_READ or IOFILE_MODE_WRITE\n");
        return -1;
    }

    // Get the file system object
    FSFile* fs_file = file_system_find_file(fs_module, filename);
    if (fs_file == NULL) {
        // errno already set by file system call
        return -1;
    }

    // Get a new fd that can be used for the file
    int new_fd = io_module_create_new_fd();
    
    // Associate the fd with a file object with malloc setting ENOMEM
    io_file_hash_table_new_file(io_module->hash_table, new_fd, mode_type, fs_file);

    return new_fd;
}

// The API call to close the given file descriptor
int io_close(int fd) {
    // Remove the fd - IOFile association from the hash table
    int removed_io_file = io_file_hash_table_remove_file(io_module->hash_table, fd);
    if (!removed_io_file) {
        // errno set by io_file_hash_table_remove_file
        return -1;
    }
    
    // Could potentially cause ENOMEM but we don't want to put the fd - IOFile pair back in the hashtable
    // because over time it could potentially degrade hashtable look up times
    free_list_push(io_module->free_list, fd);

    return 0;
}

// Read count many bytes from the IOFile pointed to by the given fd
ssize_t io_read(int fd, char* buf, size_t count) {
    IOFile* io_file = io_file_hash_table_get_file(io_module->hash_table, fd);
    if ((io_file->mode_type & IOFILE_MODE_READ) == 0) {
        errno = EROFS;
        return -1;
    }

    if (io_file == NULL)
        // errno is set by io_file_hash_table_get_file
        return -1;

    // Get the file in the file system
    FSFile* fs_file = io_file->fs_file;
    
    // If you've already read all the bytes don't read any more
    if (io_file->cursor_pos >= fs_file->size)
        return 0;

    // Make read a bit safer
    ssize_t available_bytes = fs_file->size - io_file->cursor_pos;
    ssize_t bytes_read = available_bytes;
    if (count < available_bytes)
        bytes_read = count;

    // Copy the bytes over        
    strncpy(buf, (fs_file->data + io_file->cursor_pos), available_bytes);
        
    io_file->cursor_pos += bytes_read;

    return bytes_read;
}

/* Some IO Tests */
/*
    Description: 4 file descriptors are requested from the system, 2 are freed and another 2 are requested
    Expected Result: The file descriptors of the 2 files which were closed should be reused for the new files
*/
int test_reuse() {
    printf("\n==========\ntest_reuse\n==========\n");
    // Set up a simple file system environment with a few files store as files in a linked list
    int fs_init = fs_environment_init();
    if (!fs_init) {
        fprintf(stderr, "ERROR: Unable to initialize file system\n");
        return 1;
    }

    // Initialize the IOModule
    int module_init = io_module_init();
    if (!module_init) {
        fprintf(stderr, "Unable to initialize IOModule\n");
        return 1;
    }

    // Module API Calls:
    int fd1 = io_open("file1.txt", IOFILE_MODE_READ);
    int fd2 = io_open("file1.txt", IOFILE_MODE_READ);
    int fd3 = io_open("file1.txt", IOFILE_MODE_READ);
    int fd4 = io_open("file1.txt", IOFILE_MODE_READ);

    int r = io_close(fd2);
    r = io_close(fd3);

    int fd5 = io_open("file2.txt", IOFILE_MODE_READ);
    printf("The fd of the first new file is: %d\n", fd5);

    int fd6 = io_open("file2.txt", IOFILE_MODE_READ);
    printf("The fd of the second new file is: %d\n", fd6);

    // Destroy the IOModule
    io_module_destory();

    // Destroy the file system environment
    fs_environment_destroy();
}

/*
    Description: Program attempts to close fd that never existed
    Expected Result: Call to io_close should return -1 and when calling perror(), 
                     should mention bad file descriptor error 
*/
int test_ebadf() {
    printf("\n==========\ntest_ebadf\n==========\n");

    // Set up a simple file system environment with a few files store as files in a linked list
    int fs_init = fs_environment_init();
    if (!fs_init) {
        fprintf(stderr, "ERROR: Unable to initialize file system\n");
        return 1;
    }

    // Initialize the IOModule
    int module_init = io_module_init();
    if (!module_init) {
        fprintf(stderr, "Unable to initialize IOModule\n");
        return 1;
    }

    // Module API Calls:
    int r = io_close(0);
    printf("The return code is: %d\n", r);
    perror("The error is");

    // Destroy the IOModule
    io_module_destory();

    // Destroy the file system environment
    fs_environment_destroy();
}

int test_read() {
    printf("\n=========\ntest_read\n=========\n");

    // Set up a simple file system environment with a few files store as files in a linked list
    int fs_init = fs_environment_init();
    if (!fs_init) {
        fprintf(stderr, "ERROR: Unable to initialize file system\n");
        return 1;
    }

    // Initialize the IOModule
    int module_init = io_module_init();
    if (!module_init) {
        fprintf(stderr, "Unable to initialize IOModule\n");
        return 1;
    }

    // Module API Calls:
    int fd = io_open("file2.txt", IOFILE_MODE_READ);
    if (fd == -1) {
        printf("Failed to open file\n");
        return 1;
    }

    char* buffer = (char*)malloc(sizeof(char) * 11);
    buffer[10] = '\0';
    
    io_read(fd, buffer, 10);
    printf("This is the files contents: %s\n", buffer);

    // Destroy the IOModule
    io_module_destory();

    // Destroy the file system environment
    fs_environment_destroy();
}

int main() {
    test_reuse();
    test_ebadf();
    test_read();

    return 0;
}