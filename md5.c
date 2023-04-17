#include "md5.h"

#define PIPE_R_END 0
#define PIPE_W_END 1
#define SHM_NAME "/shmmies"
#define SHM_SIZE 1048576
#define HASHES_SEM_NAME "/remaininghashes_sem"

pid_t wpid;
int status = 0;

int **slave2master;
int **master2slave;
int *masterRead;
pid_t *slavePids;

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );
    sem_t * remaining_hashes = sem_open(HASHES_SEM_NAME, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0); //semaphore to keep track of the remaining hashes to be printed
    void * shm_ptr; //pointer to the shared memory block
    int shm_fd = shm_initialize(&shm_ptr, SHM_NAME, SHM_SIZE); //initializes the shared memory block
    char * shm_ptr_char = (char *) shm_ptr; //keeps track of the current position in the shared memory block

    //printing of the number of files to be hashed, the name of the shared memory block (for view process) and the shm semaphor name
    printForVista(argc-1, SHM_NAME, HASHES_SEM_NAME);

    //creation of the file where the hash outputs will be written
    FILE *fileOutput = fopen("resultado", "a");
    if (fileOutput == NULL) {
        printf("Error opening file!\n");
        perror("fopen");
        exit(1);
    }
    
    sleep(3); //waiting for view to be executed (if it is executed at all)

    char * hashC[] = {"hashCalculate", NULL };
    char * envC[] = { NULL };

    if (argc < 2) {                     // if no files are sent to be hashed
        printf("Usage: %s file1 file2 ... fileN\n", argv[0]);
        return 1;
    }

    //calculate and save number of slaves to be used and initial files to be sent
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
            execve(hashC[0], hashC, envC);
        } else if (slavePids[i] == -1){
            perror("Error creando slave");
            exit(1);
        }
    }

    /*  closing of fd's of pipes for the master process: 
        close read end of the master2slave pipe (master will only write)
        close write end of slave2master pipe (master will only read) */
    for (int i = 0; i < slaveCount ; i++) {
        if (close(master2slave[i][PIPE_R_END]) == -1 || close(slave2master[i][PIPE_W_END]) == -1) {
            perror("Error cerrando pipes");
        }
    }

    //keeps track of what file is being sent to the slaves
    int argNumber = 1; //argument #1 is the first file to be hashed

    //send initial files to slaves
    for (int i = 0; i < initialFiles; i++) { 
        writeToSlave(i%slaveCount, (char *)argv[argNumber++]);
    }
    
    //keeps track of how many files have been printed
    int printedArgNumber = 0;

    //loop that finishes when all files have been sent & their hashes printed
    while(printedArgNumber != argNumber - 1){  
        //fd set that will be used in select to indicate which slaves finished their tasks
        fd_set masterReadSet;
        FD_ZERO(&masterReadSet); //empty the fd set
        for (int i = 0; i < slaveCount; i++) {
            FD_SET(masterRead[i], &masterReadSet); //add fd's of pipes to the set
        }

        //select waits for slave(s) to finish their task(s)
        int selectRet = select(masterRead[slaveCount-1] + 1, &masterReadSet, NULL, NULL, NULL);
        if (selectRet == -1) {
            perror("Error in select");
            exit(1);
        }

        //find slave that finished their task, write their output in shmem, and send them the next file
        for(int j=0; j < slaveCount; j++){
            //if slave j finished their task:
            if(slavePids[j] != -1 && FD_ISSET(masterRead[j], &masterReadSet)){
                char hashValue[256];
                int finalizedTasks = readFinalizedTasks(hashValue, masterRead[j]);   //read output of slave that finished task
                sprintf(shm_ptr_char, "%s", hashValue);     //writes output of slave that finished task to shmem
                shm_ptr_char += strlen(hashValue) + 1;   //updates pointer to shmem
                writeToFile(fileOutput, hashValue);   //writes output of slave that finished task to file
                for (int i = 0; i < finalizedTasks; i++) sem_post(remaining_hashes);  //increments semaphore for each task that was finalized                    
                printedArgNumber += finalizedTasks; //updates number of files that have been printed

                if (argNumber < argc) {                         //check if theres more files to send
                    writeToSlave(j, (char *)argv[argNumber++]);   //sending next file to slave number j
                }
            }
        }
    }

    closeMaster2SlaveWrite(slaveCount);      // close writing pipe of master, slave receives EOF

    freeMem(slaveCount);                     // free reserved memory

    // close and unlink the semaphores and shmem
    shm_uninitialize(shm_ptr, shm_fd, SHM_NAME);
    sem_close(remaining_hashes);
    sem_unlink(HASHES_SEM_NAME);

    closeFile(fileOutput); //close file

    while((wpid = waitpid(-1, &status, 0)) > 0); //wait for all slave processes to finish

    return 0;
}

/* returns amount of slaves to be used */
int calculateSlaves(int fileCount) {
    //calculating 5% of the total amount of files
    int n = ceil(0.05 * (double) fileCount);

    //if there are less than 5 files, there will be the same amount of slaves as files
    if(fileCount < 5)
        return fileCount;

    //case: more than (or =) 5 files, but 5% is less than 5 -> use floor number of 5 slaves
    if(n < 5)
        return 5;

    //case: 5% is more than 10 -> use ceiling number of 10 slaves
    if(n > 10)
        return 10;
    
    //else, use 5% of the fileCount as slaves
    return n;
}

/* returns the total number of files to be initially distributed to the slaves */
int calculateInitialFiles(int fileCount, int slaveCount) {
    // 10% of the files are initially distributed to the slaves
    int n = ceil(0.1 * (double) fileCount);
    
    // if there are less files than slaves, there was an excess of slaves created
    if (fileCount < slaveCount) {
        perror("Error: excess number of slaves created");
        exit(1);
    }
    //if 10% of the files is less than the number of slaves, then all the slaves are initially distributed one file
    if (n < slaveCount) {
        return slaveCount;
    }
    return n;
}

/* printf the neccesary information for the view process on the stdout */
void printForVista(int fileCount, char * shmName, char * semName) {
    fprintf(stdout, "%d\n", fileCount);
    fprintf(stdout, "%s\n", shmName);
    fprintf(stdout, "%s\n", semName);
    fflush(stdout); //forces the output to be written to the pipe
}

/* reserves memory based on the amount of slaves */
void allocateMem(int slaveCount) {
    // allocate memory for arrays of file descriptors
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

/* frees dynamic memory used */
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

/* reads from the specified fd, leaves value in hashValue. Returns number of hashes read */
int readFinalizedTasks(char * hashValue, int fd) {
    int bytesRead = 0;
    if((bytesRead = read(fd, hashValue, 512)) == -1){
        perror("Error read failed");
        exit(1);
    }

    hashValue[bytesRead]= '\0'; //null terminator

    int readHashes = 0;

    for (int i = 0 ; i < bytesRead ; i++) {
        if(hashValue[i] == '\n') {
            readHashes++;
        }
    }

    return readHashes;
}

/* sends the corresponding slave the path to the file it must hash */
void writeToSlave(int slaveNum, char * filePath) {
    char toRet [256];
    sprintf(toRet, "%s%s", filePath, "\n"); //adding newline to the end of the string
    if (write(master2slave[slaveNum][PIPE_W_END], toRet, strlen(toRet)) == -1) {
        perror("Error: write failed");
        exit(1);
    }
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

/* initialized shared memory */
int shm_initialize(void ** mem_pointer, char * name, size_t size) {
    //open the shared memory
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_fd < 0) {
        perror("Shared mem open error");
        exit(1);
    }

    //resizes the shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        exit(1);
    }

    //maps the shared memory into the process's available address
    void * shm_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Shared mem mapping failed");
        exit(1);
    }

    //clears the shared memory
    memset(shm_ptr, 0, SHM_SIZE);

    (*mem_pointer) = shm_ptr;
    return shm_fd;
}

/* uninitializes the shared memory */
void shm_uninitialize(void * ptr, int fd, char * name) {
    //unmaps the shared memory
    if (munmap(ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        exit(1);
    }

    close(fd); //closes the file descriptor

    shm_unlink(name); //unlinks the shared memory
}

/* appends stringToAppend to file "file" */
void writeToFile(FILE * file, char * stringToAppend) {
    if (fprintf(file, "%s", stringToAppend) < 0) {
        printf("Error writing to file\n");
        perror("Error writing to file on fprintf");
        exit(1);
    }
}

/* closes file "file" */
void closeFile(FILE * file) {
    if (fclose(file) != 0) {
        printf("Error closing file!\n");
        perror("fclose");
        exit(1);
    }
}