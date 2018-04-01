#include "include/mysemaphore.h"
/*
void modifySemaphore(int x, int id_sem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(id_sem, &operation, 1); 
}

void createSemaphore(int * id_sem, key_t key){
	*id_sem = semget (key, 1, 0666 | IPC_CREAT);
	if (*id_sem == -1){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't create semaphore!\n");
		exit (1);
	}
}

void changePermissions(int id_sem){
	union semun arg;

	arg.val = 0;
	semctl(id_sem, 0, SETVAL, &arg);
}*/