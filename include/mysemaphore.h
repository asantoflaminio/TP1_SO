#ifndef MYSEMAPHORE_H
#define MYSEMAPHORE_H

#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

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

//void modifySemaphore(int x, int id_sem);
//void createSemaphore(int * id_sem, key_t key);
void modifySemaphore(int x, int id_sem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(id_sem, &operation, 1); 
}

#endif

