#ifndef MD5_H
#define MD5_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/mman.h>

int calculateSlaves(int fileCount);
int calculateInitialFiles(int fileCount, int slaveCount);

void printForVista(int fileCount, char * shmName, char * semName);

void allocateMem(int slaveCount);
void freeMem(int slaveCount);

void createSlaves(int slaveCount);

void setPipes(int n);
void closePipes(int n);

int readFinalizedTasks(char * hashValue, int fd);
void writeToSlave(int slaveNum, char * filePath);

void printSlave( char * hashValue, pid_t slavePid, char * fileName);

void closeMaster2SlaveWrite(int n);

int shm_initialize(void ** mem_pointer, char * name, size_t size);
void shm_uninitialize(void * ptr, int fd, char * name);

void writeToFile(FILE * file, char * stringToAppend);
void closeFile(FILE * file);

#endif