#include "include/view.h"



int main(void){
	
	//ftok recibe path y nro como parametro
	key_t key;
	int id;
	int idSem;
	char* shm; //esta es mi memoria compartida. Va a ser un string de chars. 
	char *s; 
	int done = 0;
	int cant=1;
	int i=0;
	int status=1;

	key = ftok ("/home", 123); // uso un directorio que este en toda PC
	if(key == -1){
		perror("Error during key generation");
		wait(NULL);
		exit(1);
	}

	//creamos zona de memoria compartida. Hay que chequear que no sea -1.
	id = shmget(key, MYSIZE, 0777 | IPC_CREAT);
	if(id == -1){
		perror("Error while creating shared memory");
		wait(NULL);
		exit(1);
	}

	//Le indicamos al sistema que tome dirección donde fijar el segmento.
	shm = shmat(id, NULL, 0);
	if(shm == (char*) -1){
		perror("Error while taking a segment");
		wait(NULL);
		exit(1);
	}

	//esto va a ser despues para el semaforo
	idSem = semget (key, 1, 0777 | IPC_CREAT);
	if (idSem == -1)
	{
		printf("Error during semaphore creation\n");
		wait(NULL);
		exit (1);
	}



	//aca vendria la impresion de todos los hashes y el uso de semaforos

	/*while(!done){
		modifySemaphore(-1,idSem);
		if(shm[i]!= EOF){
			while(shm[i]!='!'){
				printf("%c", shm[i]); //Suponiendo que un signo de exclamacion es con lo q separo los hashes o con lo que termino el mensaje de cada archivo
			}
			printf("\n");
		}else{
			done = 1;
		}
		modifySemaphore(1,idSem);
	}*/
	/* VERSION GIULO
	while(!done) {
		modifySemaphore(-1,idSem);
		if(shm[0]!= EOF || shm[0] > x){
			if(shm[0]>x){
				while(x<shm[0]){
					if(status==1){
						printf("\x1B[32mFinished[%d]\x1B[0m",quantity++);
						status=0;
					}
					printf("%c",shm[x]);
					if(shm[x]=='\n')
						status=1;
					x++;
				}
			}
			if(shm[0]>2000){
				shm[0]=2;
				x=2;
			}
		}else{
			done=1;
		}
		modifySemaphore(1,idSem);
	}*/

	/*modifySemaphore(-1,idSem);
	//shm[1]=-1;
	modifySemaphore(1,idSem);*/

	
	//cuando terminamos liberamos todo. Chequear igual si es necesario hacerlo. 
	shmdt ((char *)shm);
	shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);

	printf("View has concluded\n");
	return 0;

}

//Tal vez convendria moverlo a otro lado
void modifySemaphore(int x,int idSem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(idSem, &operation, 1);
}

