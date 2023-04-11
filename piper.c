#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PIPE_R_END 0
#define PIPE_W_END 1
#define FILENAME_SIZE 1024

pid_t wpid;
int status = 0;

void readFinalizedTask(char * hashValue, int fd);
void printSlave( char * hashValue, pid_t slavePid, char * fileName);

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );

    char * tests[] = {"tests", NULL };
    char * envs[] = { NULL };

    if (argc < 2) {                     // si no se pasan archivos a hashear
        printf("Usage: %s file1 file2 ... fileN\n", argv[0]);
        return 1;
    }

    int n = argc-1>5? 5 : argc-1;   // si son menos de 5 archivos entonces n = cant de archivos

    pid_t slavePids [n];
    
    //pid_t masterPid = fork();

        int slave2master[n][2]; //fds de pipe que slave retorna rta   -> 0 read, 1 write
        int master2slave[n][2]; //fds de pipe que master manda archivos a slave
        int masterRead[n]; //fds de pipe que master lee de slaves (se usara para select)
        for(int i = 0; i < n; i++){
            if(pipe(slave2master[i]) == -1 || pipe(master2slave[i]) == -1){
                perror("Error creando pipe");
                exit(1);
            }
            masterRead[i] = slave2master[i][0];
        }

        //creacion (fork) de los n procesos slave
        for(int i = 0; i < n; i++){
            slavePids[i] = fork();
            if(slavePids[i] == 0){
                //SLAVE numero i
                close(slave2master[i][PIPE_R_END]); //en pipe de slave2master, cierro el read
                close(master2slave[i][PIPE_W_END]); //en pipe de master2slave, cierro el write
                dup2(master2slave[i][PIPE_R_END], 0); //reemplazo stdin por el read del pipe master2slave
                dup2(slave2master[i][PIPE_W_END], 1); //reemplazo stdout por el write del pipe slave2master
                
                //cierro los fd's de los otros pipes
                for(int j = 0; j < n; j++){
                    if(j != i){
                        close(slave2master[j][PIPE_R_END]);
                        close(slave2master[j][PIPE_W_END]);
                        close(master2slave[j][PIPE_R_END]);
                        close(master2slave[j][PIPE_W_END]);
                    }
                }
                execve("tests", tests, envs);
            } else if (slavePids[i] == -1){
                //Error creando slave
                perror("Error creando slave");
            }
        }

        int argNumber = 1; //en argumento #1 aparece el primer path a los archivos que quiero
        char * slaveCurrentFile[FILENAME_SIZE];         // me interesa saber que archivo tiene cada slave

        //envio inicial de archivos a slaves (mando solo 1 archivo, despues cambiar. Afecta a slaveCurrentFile tambien)
        for(int i = 0; i < n; i++){
            //printf("Mandando archivo %s a slave %d con pid: %i\n", argv[argNumber], i, slavePids[i]);
            int retWrite = write(master2slave[i][PIPE_W_END], argv[argNumber], strlen(argv[argNumber]));
            if (retWrite == -1) {
                perror("Error write failed\n");
                exit(1);
            }
            slaveCurrentFile[i] = (char *)argv[argNumber];
            //fwrite(argv[argNumber], 1, strlen(argv[argNumber]), master2slave[i][1]);
            argNumber++;
        }
    
        int printedArgNumber;

        while(printedArgNumber + 1 != argNumber){  
            fd_set masterReadSet;
            FD_ZERO(&masterReadSet); //vacío el set
            for (int i = 0; i < n; i++) {
                FD_SET(masterRead[i], &masterReadSet); //agrego los fds de los pipes que leo al set
            }
            //espero a que algun slave termine su tarea
            //int selectRet = select(n+1, masterRead, NULL, NULL, NULL);
            int selectRet = select(masterRead[n-1] + 1, &masterReadSet, NULL, NULL, NULL);
            if (selectRet == -1) {
                perror("Error in select\n");
                exit(1);
            }
            
            //encontrar el slave que terminó su tarea
            for(int j=0; j < n; j++){
                if(slavePids[j] != -1 && FD_ISSET(masterRead[j], &masterReadSet)){
                    char hashValue[256];
                    readFinalizedTask(hashValue, masterRead[j]);   //leo el resultado de la tarea que esta terminada
                    printSlave(hashValue, slavePids[j], slaveCurrentFile[j]);   // imprimo el archivo que se termino de procesar
                    printedArgNumber++;

                    if (argNumber < argc) {
                        //mando siguiente archivo a ese slave
                        //int writeRet = fwrite(argv[argNumber], 1, strlen(argv[argNumber]), master2slave[j][1]);
                        // printf("Mandando archivo %s a slave %d con pid: %i\n", argv[argNumber], j, slavePids[j]);
                        int writeRet = write(master2slave[j][1], argv[argNumber], strlen(argv[argNumber]));
                        if (writeRet == -1) {
                            perror("Error: write failed\n");
                            exit(1);
                        }
                        slaveCurrentFile[j]= (char *) argv[++argNumber];
                    }
                }

                //puede pasar que a la hora de hacer el for para ver que slave termino su tarea, otros hayan terminado tambien
                //entonces en el for anterior se mandaría mas de una tarea, sin chequear si ya nos quedamos sin archivos para hashear
                if(argNumber > argc){
                    printf("me llego mas de uno\n");
                    exit(1);
                }  
            }
        }

    for (int i = 0; i < n; i++) {
        close(master2slave[n][PIPE_W_END]); //cerrar los pipes de escritura del master
        close(master2slave[n][PIPE_R_END]); //cerrar los pipes de escritura del master
        close(slave2master[n][PIPE_W_END]); //cerrar los pipes de escritura del master
        close(slave2master[n][PIPE_R_END]); //cerrar los pipes de escritura del master
    }

    printf("llegue hasta aca");

    // HAY QUE CERRAR LOS PIPES AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA (por eso cuelga)    

    while((wpid = waitpid(-1, &status, 0)) > 0);
    printf("FEESHEN: %d\n", wpid);
    printf("Bofadeez + pid: %i\n", getpid());

    while (1);

    return 0;
}


void readFinalizedTask(char * hashValue, int fd) {
    int readRet = read(fd, hashValue, 256);
    if(readRet == -1){
        perror("Error read failed\n");
        exit(1);
    }
    return;
}

void printSlave( char * hashValue, pid_t slavePid, char * fileName) {
    printf("Hash value: %s del slave (PID): %i que recibio el archivo: %s\n", hashValue, slavePid, fileName);

    return;
}