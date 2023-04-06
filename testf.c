#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const * argv[]){

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("64\n");

    return 0;
}