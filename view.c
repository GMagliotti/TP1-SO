#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FUNCTION_ERROR -1
#define SHM_NAME "/shm_block"
#define SHM_SIZE 4096
#define PATH_MAX_LENGTH 4096
#define SEM_NAME "/remaininghashes_sem"

int main(int argc, char * argv[]) {
    // Check whether the shared mem block was received as argument or stdin
    size_t path_maxlen = PATH_MAX_LENGTH;
    char * shm_path = malloc(PATH_MAX_LENGTH);
    if (argc == 3) {
        strncpy(shm_path, argv[2], PATH_MAX_LENGTH);
    } else {
        ssize_t bytes_read = getline(&shm_path, &path_maxlen, stdin);
        shm_path[bytes_read-1] = '\0';
    }

    // Initialize the semaphore
    sem_t * remaining_hashes = sem_open(SEM_NAME, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, atoi(argv[1]));
    if (remaining_hashes == SEM_FAILED) {
        perror("Semaphone opening error");
        exit(1);
    }

    // Create the shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_fd < 0) {
        perror("Shared mem open error");
        exit(1);
    }

    // set the size of the shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        exit(1);
    }

    // Maps the shared memory into the process's available address
    void* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Shared mem mapping failed");
        exit(1);
    }
    memset(shm_ptr, 0, SHM_SIZE);

    sprintf((char *) shm_ptr, "One small step for man.\n");
    sprintf((char *) shm_ptr+strlen((char *)shm_ptr)+1, "One giant leap for mankind.\n");
    

    int offset = 0;
    for (int i = atoi(argv[1]); i > 0; i--) {
        sem_wait(remaining_hashes);
        printf("%s", ((char *)shm_ptr) + offset);
        offset += strlen(((char *)shm_ptr) + offset) + 1;
    }

    // Unmaps the shared memory
    if (munmap(shm_ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        exit(1);
    }

    shm_unlink(SHM_NAME);
    if ( sem_unlink(SEM_NAME) != 0 ) perror("Destruction fucked.");
    // free the possibly allocated memory for the path
    free(shm_path);
}