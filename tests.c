#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const * argv[]){

    setvbuf(stdout, NULL, _IONBF, 0);

    // recibo de stdin el path

    char * pathName = 

    char * hash = md5sum(/* string del path*/);

    // devuelvo por stdout el hash

    printf("%s", hash);

    return 0;
}