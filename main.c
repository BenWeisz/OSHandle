#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

void file_system_add_file(FileSystem* file_system, const char* filename, const char* data, size_t size) {
    FSFile* file = file_system_file_init(filename);
    if (file == NULL) {
        fprintf(stderr, "ERROR: Failed to add a file to file system\n");
        return;
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
}

// FUNCTIONS TO SET UP A BASIC ENVIRONMENT
// Set up a basic environment
FileSystem* fs_environment_init() {
    FileSystem* file_system = file_system_init();
    const char* filename1 = "file1.txt";
    const char* data1 = "Test Data1";
    file_system_add_file(file_system, filename1, data1, 10);

    const char* filename2 = "file2.txt";
    const char* data2 = "Test Data2";
    file_system_add_file(file_system, filename2, data2, 10);
}

// Tear down the file system environment
void fs_environment_destroy(FileSystem** file_system_ptr) {
    file_system_destroy(file_system_ptr);
}

/////////////////////////
/* File Descriptor API */
/////////////////////////

int main() {
    // Set up a simple file system environment with a few files store as files in a linked list
    FileSystem* fs_environment = fs_environment_init();

    // Destory the file system environment
    fs_environment_destroy(&fs_environment);

    return 0;
}