#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

pid_t wpid;
int status = 0;

int main(int argc, char const * argv[]){
    setvbuf(stdout, NULL, _IONBF, 0 );

    char * testf[] = {"testf.h", NULL };
    char * tests[] = {"tests.h", NULL };
    char * envf[] = { NULL };
    char * envs[] = { NULL };
    int pipem2s[2];
    int pipes2m[2];
    pipe(pipem2s);
    pipe(pipes2m)

    if(fork() == 0) {       // conecta el stdout de este
        //MASTER
        //cierro STDOUT, reemplazo por w-end del pipe master to slave (el que pasa paths de archivos)
        close(0);
        dup(pipem2s[0]);
        close(pipem2s[0]); //cierro el duplicado

        //cierro STDIN, reemplazo por r-end del pipe slave to master (slave pasa hash, master recibe)
        close(1);
        dup(pipes2m[1]);
        close(pipes2m[1]); //cierro el duplicado

        //cierro los que sobran a los que no deberia tener acceso el master:
        close(pipem2s[1]);
        close(pipes2m[0]);
        
        
        // close(pipefd[0]);
        // close(1);
        // dup(pipefd[1]);
        // close(pipefd[1]);
        execve("tests.h", tests, envs);
    } else if (fork() == 0) {
        //SLAVE
        //cierro STDOUT, reemplazo por w-end del pipe slave to master
        close(0);
        dup(pipes2m[0]);
        close(pipes2m[0]); //cierro duplicado

        //cierro STDIN, reemplazo por el r-end del pipe master to slave
        close(1);
        dup(pipem2s[1]);
        close(pipem2s[1]); //cierro duplicado

        //cierro los que sobran a los que no deberia tener acceso el master:
        close(pipem2s[0]);
        close(pipes2m[1]);

        // close(pipefd[1]);           // al stdin de este
        // close(0);
        // dup(pipefd[0]);
        // close(pipefd[1]);
        execve("testf.h", testf, envf);
    } else {
        close(3);
        close(4);
    }

    while((wpid = waitpid(-1, &status, 0)) > 0);

    printf("Bofadeez\n");

    return 0;
}