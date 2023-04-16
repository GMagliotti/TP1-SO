#include "view.h"

#define FUNCTION_ERROR -1
#define SHM_SIZE 65536
#define PATH_MAX_LENGTH 4096
#define SEM_NAME "/remaininghashes_sem"
#define NO_OFLAGS 0

int main(int argc, char * argv[]) {
    // Check whether the shared mem block was received as argument or stdin
    size_t path_maxlen = PATH_MAX_LENGTH;
    size_t file_count;
    char * shm_path; 
    
    if ((shm_path = malloc(PATH_MAX_LENGTH)) == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    if (argc == 3) {
        strncpy(shm_path, argv[2], PATH_MAX_LENGTH);
        file_count = atoi(argv[1]);
    } else {
        char * tempstr = NULL;
        size_t maxnum = 10;
        getline(&tempstr, &maxnum, stdin);
        file_count = atoi(tempstr);
        ssize_t bytes_read = getline(&shm_path, &path_maxlen, stdin);
        shm_path[bytes_read-1] = '\0';
    }

    void * shm_ptr = NULL;
    int shm_fd = shm_map(&shm_ptr, shm_path);

    int offset = 0; 

    sem_t * remaining_hashes = sem_open(SEM_NAME, NO_OFLAGS);
    if (remaining_hashes == SEM_FAILED) {
        perror("Semafucked opening error");
        exit(1);
    }

    for (int i = 0; i < file_count; i++) {
        sem_wait(remaining_hashes);
        printf("%s", (char *)shm_ptr+offset);
        offset += strlen((char *)shm_ptr+offset)+1;
    }

    shm_uninitialize(shm_ptr, shm_fd);
    sem_close(remaining_hashes);
    free(shm_path);

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