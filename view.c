#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define MYSIZE 10000 //no se por ahora le puse este size

int main(void){

	//ftok recibe path y nro como parametro
	key_t key;
	int id;
	int idSem;
	char* shm;
	char *s; 

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
	if (id_sem == -1)
	{
		printf("Error during semaphore creation\n");
		wait(NULL);
		exit (1);
	}



	//aca vendria la impresion de todos los hashes y el uso de semaforos
	//for(s = shm; )


	//cuando terminamos liberamos todo. Chequear igual si es necesario hacerlo. 
	shmdt ((char *)s);
	shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);

	printf("View has concluded");
	return 0;

}