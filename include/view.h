#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "styles.h"
#include "mySemaphore.h"
#include "mySharedMemory.h"

#define MYSIZE 30000

/* Start the execution of the program, once the entry 
through the command line was correct. Then writes in stdout
what is in shared memory. */
void start(int);

/* Generates an IPC key. */
key_t generateKey(int);
