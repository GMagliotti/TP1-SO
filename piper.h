#ifndef PIPER_H
#define PIPER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

int calculateSlaves(int fileCount);

void allocateMem(int slaveCount);
void freeMem(int slaveCount);

void readFinalizedTask(char * hashValue, int fd);
void writeToSlave(int slaveNum, char * filePath);

void printSlave( char * hashValue, pid_t slavePid, char * fileName);

void closeMaster2SlaveWrite(int n);

#endif