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
    int pipefd[2];

    pipe(pipefd);

    if(fork() == 0) {       // conecta el stdout de este
        close(pipefd[0]);
        close(1);
        dup(pipefd[1]);
        close(pipefd[1]);
        execve("testf.h", testf, envf);
    } else if (fork() == 0) {
        close(pipefd[1]);           // al stdin de este
        close(0);
        dup(pipefd[0]);
        close(pipefd[1]);
        execve("tests.h", tests, envs);
    } else {
        close(3);
        close(4);
    }

    while((wpid = waitpid(-1, &status, 0)) > 0);

    printf("Bofadeez\n");

    return 0;
}