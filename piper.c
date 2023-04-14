#include "piper.h"

#define PIPE_R_END 0
#define PIPE_W_END 1

pid_t wpid;
int status = 0;

int **slave2master;
int **master2slave;
int *masterRead;
pid_t *slavePids;

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );

    char * tests[] = {"tests", NULL };
    char * envs[] = { NULL };

    if (argc < 2) {                     // if no files are sent to be hashed
        printf("Usage: %s file1 file2 ... fileN\n", argv[0]);
        return 1;
    }

    const int slaveCount = calculateSlaves(argc-1);
    const int initialFiles = calculateInitialFiles(argc-1, slaveCount);
    
    allocateMem(slaveCount);                     // allocates memory based on the amount of slaves

    setPipes(slaveCount);                        // creates the 2 * (slaveCount) pipes that will communicate master - slave

    //creation of slaveCount amount of slaves (fork)
    for(int i = 0; i < slaveCount; i++){
        slavePids[i] = fork();
        if(slavePids[i] == 0){
            //SLAVE number i
            dup2(master2slave[i][PIPE_R_END], 0); //replace stdin with read from master2slave pipe
            dup2(slave2master[i][PIPE_W_END], 1); //replace stdout with write from slave2master pipe
            
            //closing of fd's of pipes for the slave processes
            closePipes(slaveCount);

            //run the slaves
            execve("tests", tests, envs);
        } else if (slavePids[i] == -1){
            perror("Error creando slave");
            exit(1);
        }
    }

    // cerrar el ???
    for (int i = 0; i < slaveCount ; i++) {
        if (close(master2slave[i][PIPE_R_END]) == -1 || close(slave2master[i][PIPE_W_END]) == -1) {
            perror("Error cerrando pipes");
        }
    }

    int argNumber = 1; //argument #1 is the first file to be hashed

    //send initial files to slaves
    for (int i = 0; i < initialFiles; i++) { 
        writeToSlave(i%slaveCount, (char *)argv[argNumber++]);
    }
    
    //keeps track of how many files have been printed
    int printedArgNumber = 0;

    while(printedArgNumber != argNumber - 1){  
        fd_set masterReadSet;
        FD_ZERO(&masterReadSet); //empty the fd set
        for (int i = 0; i < slaveCount; i++) {
            FD_SET(masterRead[i], &masterReadSet); //add fd's of pipes to the set
        }
        //wait until a slave finishes their task
        int selectRet = select(masterRead[slaveCount-1] + 1, &masterReadSet, NULL, NULL, NULL);
        if (selectRet == -1) {
            perror("Error in select");
            exit(1);
        }

        //find slave that finished their task, print their output
        for(int j=0; j < slaveCount; j++){
            if(slavePids[j] != -1 && FD_ISSET(masterRead[j], &masterReadSet)){
                char hashValue[256];
                readFinalizedTask(hashValue, masterRead[j]);   //read output of slave that finished task
                printf("%s", hashValue);                            //print output of slave that finished task
                printedArgNumber++;

                if (argNumber < argc) {                         //check if theres more files to send
                    writeToSlave(j, (char *)argv[argNumber++]);   //sending next file to slave number j
                }
            }

            // nunca paso esto, TO DO formalizar
            if(argNumber > argc){
                printf("me llego mas de uno\n");
                exit(1);
            }  
        }
    }

    closeMaster2SlaveWrite(slaveCount);      // close writing pipe of master, slave receives EOF

    freeMem(slaveCount);                     // free reserved memory

    while((wpid = waitpid(-1, &status, 0)) > 0);

    printf("Bofadeez + pid: %i\n", getpid());

    return 0;
}

/* returns amount of slaves to be used */
int calculateSlaves(int fileCount) {
    if (fileCount < 5) {
        return fileCount;
    }
    return 5;
}

//returns the total number of files to be initially distributed to the slaves
int calculateInitialFiles(int fileCount, int slaveCount) {
    int n = ceil(0.1 * (double) fileCount);
    
    if (fileCount < slaveCount) {
        perror("Error: excess number of slaves created");
        exit(1);
    }
    if (n < slaveCount) {
        return slaveCount;
    }
    return n;
}

/* reserves memory based on the amount of slaves */
void allocateMem(int slaveCount) {
    // allocate memory for arrays
    slave2master = (int **) malloc(slaveCount * sizeof(int *));
    master2slave = (int **) malloc(slaveCount * sizeof(int *));
    masterRead = (int *) malloc(slaveCount * sizeof(int));
    slavePids = (pid_t *) malloc(slaveCount * sizeof(pid_t));

    // check if malloc() succeeded
    if (slave2master == NULL || master2slave == NULL) {
        // handle error
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }

    // allocate memory for each sub-array
    for (int i = 0; i < slaveCount; i++) {
        slave2master[i] = (int *) malloc(2 * sizeof(int));
        master2slave[i] = (int *) malloc(2 * sizeof(int));

        // check if malloc() succeeded
        if (slave2master[i] == NULL || master2slave[i] == NULL) {
            // handle error
            fprintf(stderr, "Failed to allocate memory\n");
            exit(1);
        }
    }
    return;
}

/* frees dinamic memory used */
void freeMem(int slaveCount) {
    // free memory when done
    for (int i = 0; i < slaveCount; i++) {
        free(slave2master[i]);
        free(master2slave[i]);
    }
    free(slave2master);
    free(master2slave);
    free(masterRead);
    free(slavePids);

    return;
}

/* creates the 2 * (slaveCount) pipes that will communicate master - slave */
void setPipes(int n) {
    for(int i = 0; i < n; i++){     
        if(pipe(slave2master[i]) == -1 || pipe(master2slave[i]) == -1){
            perror("Error creando pipe");
            exit(1);
        }
        masterRead[i] = slave2master[i][0];
    }
}

/* closes all the pipes. Child processes must call this function */
void closePipes(int n) {
    for(int j = 0; j < n; j++){
            if (close(slave2master[j][PIPE_R_END]) == -1|| close(slave2master[j][PIPE_W_END]) == -1 ||
            close(master2slave[j][PIPE_R_END]) == -1 || close(master2slave[j][PIPE_W_END]) == -1) {
                perror("Close");
                exit(1);
            }
    }
}

/* reads from the specified fd, leaves value in hashValue*/
void readFinalizedTask(char * hashValue, int fd) {
    if(read(fd, hashValue, 256) == -1){
        perror("Error read failed");
        exit(1);
    }
    return;
}

/* sends the corresponding slave the path to the file it must hash */
void writeToSlave(int slaveNum, char * filePath) {
    char toRet [256];
    sprintf(toRet, "%s%s", filePath, "\n");
    if (write(master2slave[slaveNum][PIPE_W_END], toRet, strlen(toRet)) == -1) {
        perror("Error: write failed");
        exit(1);
    }
}

/* prints slave output */
void printSlave( char * hashValue, pid_t slavePid, char * fileName) {
    printf("Hash value: %s del slave (PID): %i que recibio el archivo: %s\n", hashValue, slavePid, fileName);
    return;
}

/* closes master's writing pipes, sending EOF */
void closeMaster2SlaveWrite(int n) {
    for (int i = 0; i < n; i++) {
        if (close(master2slave[i][PIPE_W_END]) == -1) { //closing of master's writing pipes    
            perror("Close");
            exit(1);
        }
    }
}