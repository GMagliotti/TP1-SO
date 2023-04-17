#include "vista.h"

#define FUNCTION_ERROR -1
#define SHM_SIZE 65536
#define PATH_MAX_LENGTH 4096
#define NO_OFLAGS 0

int main(int argc, char * argv[]) {
    // Check whether the shared mem block was received as argument or stdin
    size_t path_maxlen = PATH_MAX_LENGTH;
    size_t file_count;
    char * shm_name, * sem_name; 

    /* allocate memory for the shared memory and the semaphor */
    if ((shm_name = malloc(PATH_MAX_LENGTH)) == NULL || (sem_name = malloc(PATH_MAX_LENGTH)) == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    if (argc != 1 && argc != 4 ) {
        printf("Usage: %s fileCount shmName hashSemName\n", argv[0]);
        return 1;
    }

    if (argc == 4) {        // if it recieves the parameters through stdin
        strncpy(sem_name, argv[3], PATH_MAX_LENGTH);
        strncpy(shm_name, argv[2], PATH_MAX_LENGTH);
        file_count = atoi(argv[1]);
    } else {                // if it is called with the application process though a pipe
        char * tempstr = NULL;
        size_t maxnum = 10;
        getline(&tempstr, &maxnum, stdin);
        file_count = atoi(tempstr);
        ssize_t bytes_read = getline(&shm_name, &path_maxlen, stdin);
        shm_name[bytes_read-1] = '\0';
        bytes_read = getline(&sem_name, &path_maxlen, stdin);
        sem_name[bytes_read-1] = '\0';
    }

    // acceses the shared memory
    void * shm_ptr = NULL;
    int shm_fd = shm_map(&shm_ptr, shm_name);

    int offset = 0; 

    // acceses the semaphor
    sem_t * remaining_hashes = sem_open(sem_name, NO_OFLAGS);
    if (remaining_hashes == SEM_FAILED) {
        perror("Semafricked opening error");
        exit(1);
    }


    // prints the contents of the shared memory as they're uploaded
    for (int i = 0; i < file_count; i++) {
        sem_wait(remaining_hashes);
        printf("%s", (char *)shm_ptr+offset);
        offset += strlen((char *)shm_ptr+offset)+1;
    }

    // closes the shared memory and semaphor (but does not destroy), and frees the allocated memory
    shm_uninitialize(shm_ptr, shm_fd);
    sem_close(remaining_hashes);
    free(shm_name);
    free(sem_name);

    return 0;
}

/**
 * @brief Opens (but does not create) a shared memory block and maps it to the writeable memory 
 * 
 * @param mem_pointer The address in which the shared memory start address will be saved
 * @param name The name of the shared memory segment
 * @return int - The file descriptor of the shared memory block
 */
int shm_map(void ** mem_pointer, char * name) {
    int shm_fd = shm_open(name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_fd < 0) {
        perror("Shared mem open error");
        exit(1);
    }

    void* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Shared mem mapping failed");
        exit(1);
    }

    (*mem_pointer) = shm_ptr;
    return shm_fd;
}

/**
 * @brief Closes (but does not destroy) an open shared memory block
 * 
 * @param ptr The pointer to the address which the shared memory is mapped to
 * @param fd The file descriptor of the shared memory block
 */
void shm_uninitialize(void * ptr, int fd) {
    // Unmaps the shared memory
    if (munmap(ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        exit(1);
    }

    close(fd);
}