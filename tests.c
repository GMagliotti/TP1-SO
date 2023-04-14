#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 1024
#define HASH_MD5_SIZE 32

int main() {
    char filePath[BUF_SIZE];
    char hexHash[HASH_MD5_SIZE + 1];
    int bytesRead;

    //sem_t *initialSemaphore = sem_open("/initialSemaphore", O_CREAT, S_IRWXU, 0);

    while ((bytesRead = read(STDIN_FILENO, filePath, BUF_SIZE)) > 0) {
        //sem_post(initialSemaphore);
        filePath[bytesRead] = '\0'; // null terminated

        // calculate hash
        char cmd[BUF_SIZE+12];
        sprintf(cmd, "md5sum %s", filePath);

        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            printf("Error: Could not execute command %s\n", cmd);
            continue;
        }
        if (fscanf(fp, "%32s", hexHash) != 1) {
            printf("Error: Could not read hash from output of command %s\n", cmd);
            pclose(fp);
            continue;
        }
        pclose(fp);

        char pidString[BUF_SIZE];
        sprintf(pidString, "%d", getpid());

        char toWrite[BUF_SIZE * 4];
        sprintf(toWrite, "Hash value: %s del slave (PID): %s que recibio el archivo: %s\n",
                hexHash, pidString, filePath);
        
        
        if (write(STDOUT_FILENO, toWrite, strlen(toWrite)+1) < 0) {
            printf("Error: Could not write hash to stdout\n");
            break;
        }
    }

    if (bytesRead == 0) {
        exit(0);
    } else {
        perror("Error reading file");
        exit(1);
    }

    return 0;
}