#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define MYSIZE 10000 //no se por ahora le puse este size  tmbn definido en applciation.h

void modifySemaphore(int x,int idSem);