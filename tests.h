#ifndef TESTS_H
#define TESTS_H

#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

void processInput(char *input);
void calculateHash(char * hexHash, char *token);
void generateOut(char * toWrite, char * hexHash, char * token);

#endif