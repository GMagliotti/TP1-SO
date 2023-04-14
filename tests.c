// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

//receives filepath, calculates hash, adds its PID and file name, writes to stdout (slave2master pipe)
void calculateHash(char *token) {
    char hexHash[HASH_MD5_SIZE + 1];
    char cmd[BUF_SIZE+12];
    sprintf(cmd, "md5sum %s", token); 

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Error: Could not execute command %s\n", cmd);
        return;
    }
    if (fscanf(fp, "%32s", hexHash) != 1) {
        printf("Error: Could not read hash from output of command %s\n", cmd);
        pclose(fp);
        return;
    }
    pclose(fp);

    char pidString[BUF_SIZE]; 
    sprintf(pidString, "%d", getpid()); //stores pid in string pidString

    char toWrite[BUF_SIZE * 4];
    //concatenates all relevant information to write to stdout
    sprintf(toWrite, "Hash value: %s from slave (PID): %s of the file: %s\n",
            hexHash, pidString, token);

    //writes to stdout
    if (write(STDOUT_FILENO, toWrite, strlen(toWrite)+1) < 0) {
        printf("Error: Could not write hash to stdout\n");
        return;
    }
}

//receives input read in main function, will parse it and send filenames 
void processInput(char *input) {
    char token[256] = "";
    int tokenPos = 0;
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] == '\n') {
            token[tokenPos] = '\0'; // null terminated
            calculateHash(token);
            // reset token
            memset(token, 0, sizeof(token));
            tokenPos = 0;
        } else {
            //copy each char to token
            token[tokenPos++] = input[i];
        }
    }
}

int main() {
    char filePath[BUF_SIZE];
    int bytesRead;

    //reads from pipe (master2slave), there may be more than one file read, they are separated by '\n'
    while ((bytesRead = read(STDIN_FILENO, filePath, BUF_SIZE)) > 0) {
        filePath[bytesRead] = '\0'; // null terminated
        processInput(filePath);
    }

    if (bytesRead == 0) {
        exit(0);
    } else {
        perror("Error reading file");
        exit(1);
    }

    return 0;
}