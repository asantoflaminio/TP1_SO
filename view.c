#include "include/view.h"



int main(void){
	
	printf("Starting view process...\n");
	
	key_t key;
	int id_shmem;
	int id_sem;
	char *shm;
	int done = 0;

	key = ftok("/home", 123); 
	if(key == -1){
		perror("[ERROR!] Couldn't generate the key!\n");
		exit(1);
	}

	id_shmem = shmget(key, MYSIZE, 0777);
	if(id_shmem < 0){
		perror("[ERROR!] Couldn't get the identifier of the segment!\n");
		exit(1);
	}
	
	//Le indicamos al sistema que tome dirección donde fijar el segmento.
	shm = (char*) shmat(id_shmem, 0, 0);
	if(shm == (char*) -1){
		perror("[ERROR!] Couldn't take the shared segment!\n");
		exit(1);
	}

	//esto va a ser despues para el semaforo
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
			printf("--- From view: ");
			while(*s != '|'){
				printf("%c", *s);
				s++;
			} 
			s++;
			printf("----\n");
		}


		modifySemaphore(1,id_sem);
	}



	
    printf("Finishing view process...\n");
	return 0;

}


void modifySemaphore(int x,int idSem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(idSem, &operation, 1);
}
