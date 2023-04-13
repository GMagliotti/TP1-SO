#include "piper.h"

#define PIPE_R_END 0
#define PIPE_W_END 1
#define FILENAME_SIZE 1024

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

    if (argc < 2) {                     // si no se pasan archivos a hashear
        printf("Usage: %s file1 file2 ... fileN\n", argv[0]);
        return 1;
    }

    const int n = calculateSlaves(argc-1);
    
    allocateMem(n);                     // reserva la memoria en base a n, la cantidad de slaves

    setPipes(n);                        // crea los 2 * n pipes que se van a comunicar master - slave[]

    //creacion (fork) de los n procesos slave
    for(int i = 0; i < n; i++){
        slavePids[i] = fork();
        if(slavePids[i] == 0){
            //SLAVE numero i
            dup2(master2slave[i][PIPE_R_END], 0); //reemplazo stdin por el read del pipe master2slave
            dup2(slave2master[i][PIPE_W_END], 1); //reemplazo stdout por el write del pipe slave2master
                
            //cierro los fd's de los pipes para el slave
            closePipes(n);

            execve("tests", tests, envs);
        } else if (slavePids[i] == -1){
            perror("Error creando slave");
            exit(1);
        }
    }

    // cerrar el ???
    for (int i = 0; i < n ; i++) {
        if (close(master2slave[i][PIPE_R_END]) == -1 || close(slave2master[i][PIPE_W_END]) == -1) {
            perror("Error cerrando pipes");
        }
    }

    int argNumber = 1; //en argumento #1 aparece el primer path a los archivos que quiero
    char * slaveCurrentFile[FILENAME_SIZE];         // me interesa saber que archivo tiene cada slave

    //envio inicial de archivos a slaves (mando solo 1 archivo, despues cambiar. Afecta a slaveCurrentFile tambien)
    for(int i = 0; i < n; i++){
        writeToSlave(i, (char *)argv[argNumber]);
        slaveCurrentFile[i] = (char *)argv[argNumber];
        argNumber++;
    }
    
    int printedArgNumber = 0;

    while(printedArgNumber != argNumber - 1){  
        fd_set masterReadSet;
        FD_ZERO(&masterReadSet); //vacío el set
        for (int i = 0; i < n; i++) {
            FD_SET(masterRead[i], &masterReadSet); //agrego los fds de los pipes que leo al set
        }
        //espero a que algun slave termine su tarea
        int selectRet = select(masterRead[n-1] + 1, &masterReadSet, NULL, NULL, NULL);
        if (selectRet == -1) {
            perror("Error in select");
            exit(1);
        }
            
        //encontrar el slave que terminó su tarea
        for(int j=0; j < n; j++){
            if(slavePids[j] != -1 && FD_ISSET(masterRead[j], &masterReadSet)){
                char hashValue[256];
                readFinalizedTask(hashValue, masterRead[j]);   //leo el resultado de la tarea que esta terminada
                printSlave(hashValue, slavePids[j], slaveCurrentFile[j]);   // imprimo el archivo que se termino de procesar
                printedArgNumber++;

                if (argNumber < argc) {                         // verifica si quedan archivos por mandar
                    writeToSlave(j, (char *)argv[argNumber]);   //mando siguiente archivo al slave j
                    slaveCurrentFile[j]= (char *) argv[argNumber++];
                }
            }

            // nunca paso esto, TODO formalizar
            if(argNumber > argc){
                printf("me llego mas de uno\n");
                exit(1);
            }  
        }
    }

    closeMaster2SlaveWrite(n);      // cierra el pipe de escritura del master, slave recibe un EOF

    freeMem(n);                     // libera la memoria reservada

    while((wpid = waitpid(-1, &status, 0)) > 0);

    printf("Bofadeez + pid: %i\n", getpid());

    return 0;
}

/* devuelve la cantidad de slaves */
int calculateSlaves(int fileCount) {
    if (fileCount < 5) {
        return fileCount;
    }
    return 5;
}

/* reserva memoria en base a la cantidad de slaves */
void allocateMem(int slaveCount) {
    // allocate memory for arrays
    slave2master = (int **) malloc(slaveCount * sizeof(int *));
    master2slave = (int **) malloc(slaveCount * sizeof(int *));
    masterRead = (int *) malloc(slaveCount * sizeof(int));
    slavePids = (pid_t *) malloc(slaveCount * sizeof(pid_t));

    // allocate memory for each sub-array
    for (int i = 0; i < slaveCount; i++) {
        slave2master[i] = (int *) malloc(2 * sizeof(int));
        master2slave[i] = (int *) malloc(2 * sizeof(int));
    }
    return;
}

/* libera la memoria dinamica utilizada */
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

/* crea los pares de pipes que comunican master - slave[] */
void setPipes(int n) {
    for(int i = 0; i < n; i++){     
        if(pipe(slave2master[i]) == -1 || pipe(master2slave[i]) == -1){
            perror("Error creando pipe");
            exit(1);
        }
        masterRead[i] = slave2master[i][0];
    }
}

/* cierra todos los pipes (lo deben llamar los hijos) */
void closePipes(int n) {
    for(int j = 0; j < n; j++){
            if (close(slave2master[j][PIPE_R_END]) == -1|| close(slave2master[j][PIPE_W_END]) == -1 ||
            close(master2slave[j][PIPE_R_END]) == -1 || close(master2slave[j][PIPE_W_END]) == -1) {
                perror("Close");
                exit(1);
            }
    }
}

/* realiza la lectura del fd solicitado, deja el resultado en hashValue */
void readFinalizedTask(char * hashValue, int fd) {
    if(read(fd, hashValue, 256) == -1){
        perror("Error read failed");
        exit(1);
    }
    return;
}

/* manda al esclavo correspondiente el path del archivo */
void writeToSlave(int slaveNum, char * filePath) {
    if (write(master2slave[slaveNum][PIPE_W_END], filePath, strlen(filePath)) == -1) {
        perror("Error: write failed");
        exit(1);
    }
}

/* imprime el valor devuelto por el slave */
void printSlave( char * hashValue, pid_t slavePid, char * fileName) {
    printf("Hash value: %s del slave (PID): %i que recibio el archivo: %s\n", hashValue, slavePid, fileName);
    return;
}

/* cierra los pipes de escritura del master, mandando un EOF */
void closeMaster2SlaveWrite(int n) {
    for (int i = 0; i < n; i++) {
        if (close(master2slave[i][PIPE_W_END]) == -1) { //cerrar los pipes de escritura del master    
            perror("Close");
            exit(1);
        }
    }
}