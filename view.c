#include "include/view.h"

#include "include/mysemaphore.h"

int 
main(int argc, char* argv[]){
	
	printf("Starting view process...\n");
	
	key_t key;
	int id_shmem;
	
	int id_sem;

	char * shm;
	int done = 0;
	
	if(argc != 2){
		printf("The arguments were wrong. Correct format: hash <directory>\n");
		return -1;
	}


	key = ftok("/home", atoi(argv[1]));
	if(key == -1){
		perror("[ERROR!] Couldn't generate the key!\n");
		exit(1);
	}
	
	id_shmem = shmget(key, MYSIZE, 0777);
	if(id_shmem < 0){
		perror("[ERROR!] Couldn't get the identifier of the segment!\n");
		exit(1);
	}
	
	shm = (char*) shmat(id_shmem, 0, 0);
	if(shm == (char*) -1){
		perror("[ERROR!] Couldn't take the shared segment!\n");
		exit(1);
	}
	
	id_sem = semget (key, 1, 0600 | IPC_CREAT);
	if (id_sem == -1)
	{
		perror("[ERROR!] Couldn't create semaphore!\n");
		exit (1);
	}
	

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


	shmdt(shm);
	shmctl(id_shmem,  IPC_RMID, 0);
	
    	printf("Finishing view process...\n");
	
	return 0;

}


