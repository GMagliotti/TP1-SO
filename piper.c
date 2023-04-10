#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pid_t wpid;
int status = 0;

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );

    char * tests[] = {"tests", NULL };
    char * envs[] = { NULL };

    //hacer algun calculo con la cantidad de archivos que me pasan para ver cuantos slaves creo
    int n = 5;
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
                close(slave2master[i][0]); //en pipe de slave2master, cierro el read
                close(master2slave[i][1]); //en pipe de master2slave, cierro el write
                dup2(master2slave[i][0], 0); //reemplazo stdin por el read del pipe master2slave
                dup2(slave2master[i][1], 1); //reemplazo stdout por el write del pipe slave2master
                
                //cierro los fd's de los otros pipes
                for(int j = 0; j < n; j++){
                    if(j != i){
                        close(slave2master[j][0]);
                        close(slave2master[j][1]);
                        close(master2slave[j][0]);
                        close(master2slave[j][1]);
                    }
                }
                execve("tests", tests, envs);
            } else if (slavePids[i] == -1){
                //Error creando slave
                perror("Error creando slave");
            }
        }

        int pendingTasks[n]; //cantidad de tareas pendientes por cada slave
        int argNumber = 1; //en argumento #1 aparece el primer path a los archivos que quiero
        //envio inicial de archivos a slaves (mando solo 1 archivo, despues cambiar)
        for(int i = 0; i < n; i++){
            //printf("Mandando archivo %s a slave %d con pid: %i\n", argv[argNumber], i, slavePids[i]);
            int retWrite = write(master2slave[i][1], argv[argNumber], strlen(argv[argNumber]));
            if (retWrite == -1) {
                perror("Error write failed\n");
                exit(1);
            }
            //fwrite(argv[argNumber], 1, strlen(argv[argNumber]), master2slave[i][1]);
            argNumber++;
            pendingTasks[i] = 1;
        }

    
        while(argNumber < argc){  
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
                    //leo el resultado de la tarea que esta terminada
                    char hashValue[256];
                    int readRet = read(masterRead[j], hashValue, 256);
                    if(readRet == -1){
                        perror("Error read failed\n");
                        exit(1);
                    }
                    printf("Hash value: %s del slave: %i\n", hashValue, slavePids[j]); //testeo
                    
                    //mando siguiente archivo a ese slave
                    //int writeRet = fwrite(argv[argNumber], 1, strlen(argv[argNumber]), master2slave[j][1]);
                    printf("Mandando archivo %s a slave %d con pid: %i\n", argv[argNumber], j, slavePids[j]);
                    int writeRet = write(master2slave[j][1], argv[argNumber], strlen(argv[argNumber]));
                    argNumber++;
                    if (writeRet == -1) {
                        perror("Error write failed\n");
                        exit(1);
                    }
                }
                //puede pasar que a la hora de hacer el for para ver que slave termino su tarea, otros hayan terminado tambien
                //entonces en el for anterior se mandaría mas de una tarea, sin chequear si ya nos quedamos sin archivos para hashear
                if(argNumber >= argc){
                    printf("me llego mas de uno\n");
                    break;
                }
                
            }
        }

    while((wpid = waitpid(-1, &status, 0)) > 0);
    printf("Bofadeez + pid: %i\n", getpid());

    return 0;
}
