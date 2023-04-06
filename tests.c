#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char const * argv[]) {
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    linelen = getline(&line, &linecap, stdin);

    printf("Process ID: %d realizo el hash ", getpid());
    fwrite(line, linelen, 1, stdout);


    return getpid()*2;      // esto no va a ir a ningun lado
}