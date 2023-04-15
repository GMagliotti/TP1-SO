#ifndef VIEW_H
#define VIEW_H

#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>

int shm_initialize(void **, char *);
void shm_uninitialize(void *, int);

#endif
