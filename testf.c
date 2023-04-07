#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

/* 
    determinar la diferencia entre un testf y un piper, ¿necesitamos que sean distintos procesos?
    todos los procesos hijos tendran conectados el stdin al stdout de este y todos los stdout a el stdin de este
    ver como hacer para que solo un hijo reciba el nombre del archivo que salga por stdout, y mediante select() este este recibiendo todos los impits sincronicamente
*/
int main(int argc, char const * argv[]) {
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    linelen = getline(&line, &linecap, stdin);

    printf("Process ID: %d realizo el hash ", getpid());
    fwrite(line, linelen, 1, stdout);


    return getpid()*2;      // esto no va a ir a ningun lado?
}