#include "tests.h"

#define BUF_SIZE 1024
#define HASH_MD5_SIZE 32
#define MUTEX_SEM_NAME "/semmies"

int main() {

    char *filePath = NULL;
    size_t len = BUF_SIZE;
    ssize_t bytesRead;

    sem_t * mutex = sem_open(MUTEX_SEM_NAME, 0);
    
    while((bytesRead = getline(&filePath, &len, stdin)) != -1){
        if (bytesRead != -1) {
            filePath[bytesRead-1] = '\0';
            processInput(filePath);
        } else {
            perror("Error reading file");
            free(filePath);
            exit(1);
        }
    }
    
    free(filePath);
    sem_close(mutex);

    return 0;
}


// receives input read in main function, will parse it and send filenames 
void processInput(char *input) {
    //recibo fileName en input
    char hexHash[HASH_MD5_SIZE + 1];
    calculateHash(hexHash, input);
    char toWrite[512];
    generateOut(toWrite, hexHash, input);

    //writes to stdout
    if (write(STDOUT_FILENO, toWrite, strlen(toWrite)) < 0) {
        perror("Error - Could not write hash to stdout\n");
        exit(1);
    }

}


// receives filepath, calculates hash, adds its PID and file name, writes to stdout (slave2master pipe)
void calculateHash(char * hexHash, char *token) {
    char *args[] = { "/usr/bin/md5sum", token, NULL };
    char *env[] = {"PATH=/usr/bin", NULL};
    int status;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // child process
        close(pipefd[0]);  // close read end of pipe
        dup2(pipefd[1], STDOUT_FILENO);  // redirect stdout to pipe
        close(pipefd[1]);  // close write end of pipe
        execve(args[0], args, env);
        perror("execve");  // execve returns only on error
        exit(EXIT_FAILURE);
    } else {  // parent process
        close(pipefd[1]);  // close write end of pipe
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        if (read(pipefd[0], hexHash, 32) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        hexHash[32] = '\0';  // null-terminate hash string
        close(pipefd[0]);  // close read end of pipe
    }
    return;
}


/* generates the string that will be written to stdout, and writes it. */
void generateOut(char * toWrite, char * hexHash, char * token) {
    char pidString[BUF_SIZE]; 
    sprintf(pidString, "%d", getpid()); //stores pid in string pidString
    
    //extracts file name from path
    char * fileName = strrchr(token, '/') + 1; 

    //concatenates all relevant information to write to stdout
    sprintf(toWrite, "Hash value: %s from slave (PID): %s of the file: %s\n",
            hexHash, pidString, fileName);

    return;
}