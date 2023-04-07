#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

pid_t wpid;
int status = 0;

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );

    char * testf[] = {"testf", NULL };
    char * tests[] = {"tests", NULL };
    char * envf[] = { NULL };
    char * envs[] = { NULL };
    

    if(fork() == 0) {       // conecta el stdout de este
        //MASTER
        int n = 5;
        int slave2master[n][2]; //fds de pipe que slave retorna rta   -> 0 read, 1 write
        int master2slave[n][2]; //fds de pipe que master manda archivos a slave
        for(int i = 0; i < n; i++){
            pipe(slave2master[i]);
            pipe(master2slave[i]);
        }

        //creacion (fork) de los n procesos slave
        for(int i = 0; i < n; i++){
            if(fork() == 0){
                //SLAVE numero i
                close(slave2master[i][0]); //en pipe de slave2master, cierro el read
                close(master2slave[i][1]); //en pipe de master2slave, cierro el write
                dup2(master2slave[i][0], 0); //reemplazo stdin por el read del pipe master2slave
                dup2(slave2master[i][1], 1); //reemplazo stdout por el write del pipe slave2master
                
                for(int j = 0; j < n; j++){
                    if(j != i){
                        close(slave2master[j][0]);
                        close(slave2master[j][1]);
                        close(master2slave[j][0]);
                        close(master2slave[j][1]);
                    }
                    
                }
                execve("testf", testf, envf);
            } else{
                //Error creando slave
            }
        }

    } else{
        //Error creando master
    }

    while((wpid = waitpid(-1, &status, 0)) > 0);
    while(1);
    printf("Bofadeez\n");

    return 0;
}