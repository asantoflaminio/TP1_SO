#ifndef MYSHAREDMEMORY_H
#define MYSHAREDMEMORY_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include "styles.h"

#define MYSIZE 50000

/* Creates a new segment and gets the identifier of the shared 
memory associated with the value of the argument key. Then 
calls getSharedSegment and returns its value. */
char * createSharedMemorySegment(int *, key_t);

/* Finds the segment associated with key and gets the identifier of the shared 
memory. Then calls getSharedSegment and returns its value. */
char * getSharedMemorySegmentForView(int *, key_t);

/* Attaches the shared memory segment identified by id to the address
space of the calling process. */
char * getSharedSegment(int);

/* Detach a shared memory by calling shmdt and then removes it using shmctl. */
void detachAndRemoveSharedMem(int, char *);

#endif
