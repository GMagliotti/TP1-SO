#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define BUF_SIZE 1024
#define HASH_MD5_SIZE 32

int main() {
    char filePath[BUF_SIZE];
    char hexHash[HASH_MD5_SIZE + 1];
    int bytesRead;

    while ((bytesRead = read(STDIN_FILENO, filePath, BUF_SIZE)) > 0) {
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

        // write hash to stdout
        if (write(STDOUT_FILENO, hexHash, HASH_MD5_SIZE+1) < 0) {
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