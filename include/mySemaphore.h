#ifndef MYSEMAPHORE_H
#define MYSEMAPHORE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

#include "styles.h"


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// If union is alredy defined in sys/sem.h
#else
// else, we have to define it
union semun{
    int val;  /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};
#endif

/* Modifies semaphore value according to x */
void modifySemaphore(int, int);

/* Creates a new semaphore or accesses an existing one */
void createSemaphore(int *, key_t);

/* Sets the value of the semaphore to the val member of the union. */
void changePermissions(int);

/* Removes the semaphore identifier from the system and frees the storage */
void removeSemaphore(int);

#endif

