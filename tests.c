#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>

#define BUF_SIZE 1024

int main() {
    char file_path[BUF_SIZE];
    unsigned char hash[MD5_DIGEST_LENGTH];
    char hex_hash[2*MD5_DIGEST_LENGTH + 1];
    int bytes_read;

    int i = 0;

    while (i++ < 1) {
        // read file path from stdin
        if ((bytes_read = read(STDIN_FILENO, file_path, BUF_SIZE)) <= 0) {
            break;
        }
        // file_path[bytes_read-1] = '\0'; // remove newline character

        // calculate hash
        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            printf("Error: Could not open file %s\n", file_path);
            continue;
        }
        MD5_CTX md5_ctx;
        MD5_Init(&md5_ctx);
        unsigned char buf[BUF_SIZE];
        int bytes_read;
        while ((bytes_read = fread(buf, 1, BUF_SIZE, file)) != 0) {
            MD5_Update(&md5_ctx, buf, bytes_read);
        }
        MD5_Final(hash, &md5_ctx);
        fclose(file);

        // convert hash to hexadecimal string
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sprintf(&hex_hash[i*2], "%02x", (unsigned int)hash[i]);
        }
        hex_hash[2*MD5_DIGEST_LENGTH] = '\0';

        // write hash to stdout
        if (write(STDOUT_FILENO, hex_hash, 2*MD5_DIGEST_LENGTH+1) < 0) {
            printf("Error: Could not write hash to stdout\n");
            break;
        }
    }

    return 0;
}