#include "include/mySharedMemory.h"

char * getSharedMemorySegmentForView(int * id_shmem, key_t key){
	*id_shmem = shmget(key, MYSIZE, 0777);
	if(*id_shmem < 0){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't create shared memory!");
		exit(1);
	}

	return getSharedSegment(*id_shmem);
}

char * createSharedMemorySegment(int * id_shmem, key_t key){
	*id_shmem = shmget(key, MYSIZE, 0777 | IPC_CREAT);
	if(*id_shmem < 0){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't create shared memory!");
		exit(1);
	}
	
	return getSharedSegment(*id_shmem);
}

char * getSharedSegment(int id_shmem){
	char * shm;

	shm = (char*) shmat(id_shmem, 0, 0);
	if(shm == (char*) -1){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't take the shared segment!\n");
		exit(1);
	}

	return shm;
}