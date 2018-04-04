#include "include/view.h"

int 
main(int argc, char* argv[]){
	
	if(argc != 2){
		printf("The arguments were wrong. Correct format: hash <directory>\n");
		return -1;
	} 

	printf("Starting view process...\n");
	start(atoi(argv[1]));
	printf("Finishing view process...\n");
	return 0;
}

void start(int pid){
	key_t key;
	int id_shmem;
	
	int id_sem;

	char * shm;
	int done = 0;
	
	key = generateKey(pid);
	
	printf("Taking shared memory segment...\n");
	shm = getSharedMemorySegmentForView(&id_shmem, key);

	printf("Creating semaphore...\n");
	createSemaphore(&id_sem, key);
	
	char * s = shm;

	while(!done){
		modifySemaphore(-1,id_sem);
		
		if(*s == '?')
			done = 1;
		else if(*s != '\0'){
			while(*s != '|'){
				printf("%c", *s);
				s++;
			}
			puts("");
			s++;
		}
		
		modifySemaphore(1,id_sem);
	}

	printf("Detaching and removing a shared memory segment...\n");
    detachAndRemoveSharedMem(id_shmem, shm);
}


key_t generateKey(int num){
	key_t key = ftok("/home", num);
	
	if (key == -1) {
		perror(ANSI_RED"[ERROR!] " ANSI_RESET "Couldn't generate the key!");
		exit(1);
	}

	return key;
}

