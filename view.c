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
#define SHM_SIZE 65536
#define PATH_MAX_LENGTH 4096
#define SEM_NAME "/remaininghashes_sem"
#define NO_OFLAGS 0

int shm_initialize(void **, char *);
void shm_uninitialize(void *, int);

int main(int argc, char * argv[]) {
    // Check whether the shared mem block was received as argument or stdin
    size_t path_maxlen = PATH_MAX_LENGTH;
    size_t file_count;
    char * shm_path = malloc(PATH_MAX_LENGTH);
    if (argc == 3) {
        strncpy(shm_path, argv[2], PATH_MAX_LENGTH);
        file_count = atoi(argv[1]);
    } else {
        // todo - ver porque no lee de stdin hasta que finaliza la ejecucion
        // del otro proceso, its kinda fucked up.
        char * tempstr = NULL;
        size_t maxnum = 10;
        getline(&tempstr, &maxnum, stdin);
        file_count = atoi(tempstr);
        // ssize_t bytes_read = read(STDIN_FILENO, shm_path, PATH_MAX_LENGTH);
        ssize_t bytes_read = getline(&shm_path, &path_maxlen, stdin);
        shm_path[bytes_read-1] = '\0';
    }

    void * shm_ptr = NULL;
    int shm_fd = shm_initialize(&shm_ptr, shm_path);

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

    /*
    int offset = 0;
    for (int i = atoi(argv[1]); i > 0; i--) {
        sem_wait(remaining_hashes);
        printf("%s", ((char *)shm_ptr) + offset);
        offset += strlen(((char *)shm_ptr) + offset) + 1;
    }
    */

    shm_uninitialize(shm_ptr, shm_fd);
    // shm_unlink(SHM_NAME);
    // free the possibly allocated memory for the path
    sem_close(remaining_hashes);
    free(shm_path);
}

int shm_initialize(void ** mem_pointer, char * name) {
    int shm_fd = shm_open(name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_fd < 0) {
        perror("Shared mem open error");
        exit(1);
    }

    // set the size of the shared memory object
    /*
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        exit(1);
    }
    */

    // Maps the shared memory into the process's available address
    void* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Shared mem mapping failed");
        exit(1);
    }
    //memset(shm_ptr, 0, SHM_SIZE);

    (*mem_pointer) = shm_ptr;
    return shm_fd;
}

void shm_uninitialize(void * ptr, int fd) {
    // Unmaps the shared memory
    if (munmap(ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        exit(1);
    }

    close(fd);
}