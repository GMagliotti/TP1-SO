#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#define NAMEDPIPE "./npipe"

int main (int argc, char * argv[]) {
    int named_pipe_fd = open(NAMEDPIPE, O_RDONLY);
    
    char buffer[PIPE_BUF];

    while(read(named_pipe_fd, buffer, PIPE_BUF) > 0) {
        printf("%s", buffer);
    }

    // esto va a colgar, hay que mandarle un \0 para que corte el read una vez que se imprimen todos los archivos (podria hacerse despues del waitpid desde el md5)

    return 0;
}