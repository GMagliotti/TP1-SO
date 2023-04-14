#ifndef PIPER_H
#define PIPER_H

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

int calculateSlaves(int fileCount);
int calculateInitialFiles(int fileCount, int slaveCount);

void allocateMem(int slaveCount);
void freeMem(int slaveCount);

void setPipes(int n);
void closePipes(int n);

void readFinalizedTask(char * hashValue, int fd);
void writeToSlave(int slaveNum, char * filePath);

void printSlave( char * hashValue, pid_t slavePid, char * fileName);

void closeMaster2SlaveWrite(int n);

#endif