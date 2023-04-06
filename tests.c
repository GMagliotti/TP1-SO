#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const * argv[]){

    setvbuf(stdout, NULL, _IONBF, 0);

    // recibo de stdin el path
    
    char * pathName = stdin;

    read(1, pathName, 100); //en fd=1 recibo pathname, lo guardo en var pathName, size max 100

//    char * hash = md5sum(/* string del path*/);

    // devuelvo por stdout el hash

    printf("%s", pathName);

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
