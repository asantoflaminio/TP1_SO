#include "include/view.h"



int main(void){
	
	printf("Starting view process...\n");
	//ftok recibe path y nro como parametro
	key_t key;
	int id_shmem;
	int id_sem;
	char *shm; //esta es mi memoria compartida. Va a ser un string de chars. 
	char *s; 
	int done = 0;
	int cant=1;
	int i=0;
	int status=1;

	key = ftok("/bin/ls", 123); 
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
	shm = shmat(id_shmem, 0, 0);
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
	
	//aca vendria la impresion de todos los hashes y el uso de semaforos
	modifySemaphore(-1,id_sem);
	printf(".......Writing from view.......\n");
	for(s=shm; *s != 0; s++)
		printf("%c", *s);
	modifySemaphore(1,id_sem);

	/*while(!done){
		modifySemaphore(-1,id_sem);
		if(*shm != EOF){
			while(*shm!='|'){
				printf("%c", *shm);
				shm++;
			}
			printf("\n");
		}else{
			done = 1;
		}
		modifySemaphore(1,id_sem);
	}*/
	
	/*modifySemaphore(-1,id_sem);
	//shm[1]=-1;
	modifySemaphore(1,id_sem);

	
	//cuando terminamos liberamos todo. Chequear igual si es necesario hacerlo. 
	shmdt ((char *)shm);
	shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);*/

	printf("View has concluded\n");
	return 0;

}


void modifySemaphore(int x,int idSem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(idSem, &operation, 1);
}
