#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char const * argv[]){

    setvbuf(stdout, NULL, _IONBF, 0);

    // recibo de stdin el path
    sleep(getpid()%8);
    //char * pathName = stdin;
    char pathName[256];

    int n = read(0, pathName, 256); //en fd=1 recibo pathname, lo guardo en var pathName, size max 100
    
    pathName[n] = '\0'; //agrego el \0 al final del string
    if(n == -1){
        perror("Error leyendo del pipe master2slave");
        exit(1);
    }
    else if(n == 0){
        perror("Pipe master2slave cerrado");
        exit(1);
    }
    
//    char * hash = md5sum(/* string del path*/);

    // devuelvo por stdout el hash

    n = write(1, pathName, 256); //en fd=0 escribo el hash (aca dice pathName como prueba)
    if(n == -1){
        perror("Error escribiendo en pipe slave2master");
        exit(1);
    }
    else if(n == 0){
        perror("Pipe slave2master cerrado");
        exit(1);
    }

    return 0;
}

/**********************************************************/
/*
#include <openssl/md5.h>
#include <unistd.h>
int main()
{
    int n;
    MD5_CTX c;
    char buf[512];
    ssize_t bytes;
    unsigned char out[MD5_DIGEST_LENGTH];

    MD5_Init(&c);
    bytes=read(STDIN_FILENO, buf, 512);
    while(bytes > 0)
    {
        MD5_Update(&c, buf, bytes);
        bytes=read(STDIN_FILENO, buf, 512);
    }

    MD5_Final(out, &c);

    for(n=0; n<MD5_DIGEST_LENGTH; n++)
        printf("%02x", out[n]);
    printf("\n");

    return(0);        
}
*/
