#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include "include/application.h"
#include "include/order.h"
#include "include/queue.h"
#include "include/slave.h"
#include "include/types.h"

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// La union ya está definida en sys/sem.h
#else
// Tenemos que definir la union
union semun 
{ 
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif



int 
main(int argc, char* argv[]){
	
	if (argc != 3 || (strcmp(argv[1], "hash") != 0)) {
		printf("The arguments were wrong. Format: hash <directory>\n");
		return -1;
	}

	int pid = fork();

	switch(pid){
		case -1:
			perror("[ERROR!] Couldn't fork correctly!\n");
			wait(NULL);
			exit(EXIT_FAILURE);
			break;
		case 0:
			execlp(VIEW_EXEC, VIEW_EXEC, NULL);
			perror("[ERROR!] Couldn't execute worker in forked child!\n");
			wait(NULL);
			break;
		default:
			start(argv[2]);
			break;
	}
	
	return 0;
}

void start(const char *dirname){
	printf("Starting application process...\n");
	int files;
	queue_o orderQueue;
	slaves_o * slaves;
	FILE *file;

	key_t key;
	int id_shmem;
	int id_sem;
	char *shm; 
	union semun arg;


	key = ftok("/home", 123); 
	if (key == -1) {
		perror("[ERROR!] Couldn't generate the key!\n");
		exit(1);
	}

	id_shmem = shmget(key, MYSIZE, 0777 | IPC_CREAT);
	if(id_shmem < 0){
		perror("[ERROR!] Couldn't create shared memory!\n");
		exit(1);
	}
	
	shm = (char*) shmat(id_shmem, 0, 0);
	if(shm == (char*) -1){
		perror("[ERROR!] Couldn't take the shared segment!\n");
		exit(1);
	}
	
	id_sem = semget (key, 1, 0666 | IPC_CREAT);
	if (id_sem == -1)
	{
		perror("[ERROR!] Couldn't create semaphore!\n");
		exit (1);
	}
	
	arg.val = 0;
	semctl(id_sem, 0, SETVAL, &arg);
   	modifySemaphore(1, id_sem);

	printf("Creating order queue...\n");
	orderQueue = newQueue();

	printf("Fetching files...\n");
	files = loadFiles(dirname, orderQueue, files);
	printf("%d files were fetched!\n", files);
	
	if(files == 0){
		printf("No files to process\n");
		exit(EXIT_SUCCESS);
	} else {
		
		node_o * current = orderQueue->first;
		
		while(current != NULL){
			printf("processed?: %s - ", current->order.processed? "true" : "false");
			printf("filename: %s\n", current->order.filename);
			current = current->next;
		}
		
		printf("Queue size: %d\n", orderQueue->size);
	}

	printf("Creating %d slaves..\n", SLAVES_NUM);
	slaves = createSlaves();

	int assignedOrder = 0;
	int finishOrder = 0;
	int queueSize = orderQueue->size;
	int pointer = 0;

	char **hashes = (char **)malloc(files * sizeof (char *));
	char * buff = malloc(100 * sizeof(char));
	char * curr = buff;
	int i;
	

	while(finishOrder != queueSize){ 

		if(assignedOrder != queueSize)
			orderQueue = assignWork(slaves, orderQueue, queueSize, &assignedOrder); 	

		for(i = 0; i < SLAVES_NUM; i++){
			if(slaves[i].isWorking){
				while(read(slaves[i].pipeChildToFather[0], curr, 1) == 1){
					if(*curr == '\n'){
						*curr = '\0';
						slaves[i].isWorking = false;
						finishOrder++;
						hashes[pointer] = malloc(100 * sizeof(char));
						strcpy(hashes[pointer], buff); 
						modifySemaphore(-1,id_sem);
						//puts("Entro el padre, el hijo espera"); -->Lo usamos para debuggear el semaforo
						//while(1); -->Lo usamos para debuggear el semaforo
						strcat(shm, buff);
						strcat(shm, "|"); // -->La idea es generar en memoria compartida un string largo que sea hash1|hash2|hash3 y asi..
						//puts("**El padre sale"); -->Lo usamos para debuggear el semaforo
						modifySemaphore(1,id_sem);
						curr = buff;
						pointer++;
					}
					else{
						curr++;
					}
				}
			}
		}
		
	}

	modifySemaphore(1,id_sem);
	strcat(shm,"?"); //-->Caracter donde finaliza mi string largo compartido para reconocer q tengo que salir del while(1) en view..
	modifySemaphore(-1,id_sem);

	free(buff);

	// Chequeo que se me armo en el string largo de forma correcta...
	
	/*char * s;
	printf("Shared memory: ");
	
	for(s = shm; *s != '?'; s++)
	printf("%c",*s);
	
	printf("\n");
	*/
	

	printf("Stopping slaves from working..\n");
	stopSlaves(slaves);
	

	file = fopen("my_hashes.txt", "w+");
	if (file==NULL) {
		perror("[ERROR!] File error!\n");
		exit(1);
	}

	int j = 0;
	for(j = 0; j < queueSize ; j++){
		//printf("From application: %s\n" , hashes[j]);
		fputs(hashes[j],file);
		fputs("\n",file);
		
	}

	int pid, status;
	while ((pid=waitpid(-1,&status,0)) != -1) {
        printf("Process %d finished\n",pid);
    }

    shmdt(shm);
	shmctl(id_shmem,  IPC_RMID, 0);
	semctl(id_sem, 0, IPC_RMID); // Remove the semaphore
	
    fclose(file);
    printf("Finishing application process...\n");
    
}


int loadFiles(const char *dirname, queue_o queue, int files){
	DIR *dir;
	struct dirent *dp;

	dir = opendir(dirname);
	
	if((dir = opendir(dirname)) != NULL){
		while((dp = readdir(dir)) != NULL){
			if((strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name,"..")!=0)){

				char * current = malloc(strlen(dirname) + strlen(dp->d_name) + strlen(SEPARATOR) + 1);
				strcat(strcat(strcpy(current, dirname), SEPARATOR), dp->d_name);
				
				if(strlen(current) > MAX_FILENAME){
					 perror("[ERROR!] One file exceeded our filename limit!\n");
	    			_exit(1);
				}

				if(dp->d_type == DT_DIR){
					files = loadFiles(current, queue, files);
				} else {
					order_o order;
					order.filename = current;
					order.processed = false;
					enQueue(queue, order);
					files++;
				}
			}
		}
	}

	closedir(dir);
	return files;
}


slaves_o * createSlaves(){
	int i;
	slaves_o * slaves;

	slaves = (slaves_o *)calloc(SLAVES_NUM, sizeof(slaves_o));

	if(slaves == NULL) {
	    perror("[ERROR!] Couldn't allocate space for slaves!\n");
	    wait(NULL);
	    _exit(1);
  	}


	for(i = 0; i < SLAVES_NUM; i++){
		pid_t pid;

		slaves[i].isWorking = false;

	  	if(pipe(slaves[i].pipeFatherToChild) == -1){
	        printf("[ERROR!] Couldn't open the pipe Father->Child!\n");
	    }

	    if(pipe(slaves[i].pipeChildToFather) == -1){
	        printf("[ERROR!] Couldn't open the pipe Child->Father!\n");
	    }
	   	fcntl(slaves[i].pipeChildToFather[0], F_SETFL,  O_NONBLOCK);

		//Start slave
		pid = fork();
		int flags = fcntl(slaves[i].pipeChildToFather[0], F_GETFL, 0);
		switch(pid){
			case -1:
				perror("[ERROR!] Couldn't fork correctly!\n");
				wait(NULL);
				exit(EXIT_FAILURE);
				break;
			case 0:
				//If i'm the child, execute ./slave
				dup2(slaves[i].pipeFatherToChild[0], STDIN_FILENO);
				if(dup2(slaves[i].pipeChildToFather[1], STDOUT_FILENO) == -1)
					perror("[ERROR!] Couldn't redirect stdout of child\n");
		 		close(slaves[i].pipeFatherToChild[1]);
		 		close(slaves[i].pipeChildToFather[0]);
				execlp(SLAVE_EXEC, SLAVE_EXEC, NULL);
				perror("[ERROR!] Couldn't execute worker in forked child!\n");
				wait(NULL);
				break;
			default:
				//dup2(slaves[i].pipeChildToFather[0], STDIN_FILENO);
				fcntl(slaves[i].pipeChildToFather[0], F_SETFL, flags | O_NONBLOCK);
				close(slaves[i].pipeFatherToChild[0]);
				close(slaves[i].pipeChildToFather[1]);
				break;
		}
	}

	return slaves;
	
}

queue_o assignWork(slaves_o * slaves, queue_o orderQueue, int queueSize, int * assignedOrder){
	int i, j;

	for(i = 0; i < SLAVES_NUM && queueSize != *assignedOrder; i++){	
		if(slaves[i].isWorking == false){
			for(j = 0; j < ORDERS_NUM && (*assignedOrder != queueSize) ; j++){
				if(orderQueue->first->order.processed == false){
					write(slaves[i].pipeFatherToChild[1], orderQueue->first->order.filename, strlen(orderQueue->first->order.filename));
					write(slaves[i].pipeFatherToChild[1], "|", 1);
					node_o * temp = deQueue(orderQueue);
					printf("Sending %s to slave number %d\n", temp->order.filename, i);
					slaves[i].isWorking = true;
					(*assignedOrder)++;
				} else {
					printf("[ERROR!] Trying to process a file that has already been processed!\n");
					break; //SACAR
				}

			} write(slaves[i].pipeFatherToChild[1], "", 1);
		}
	}

	return orderQueue;
}



void stopSlaves(slaves_o * slaves){
	int i;
	for(i = 0; i < SLAVES_NUM; i++){
		write(slaves[i].pipeFatherToChild[1],STOP_SLAVES,1);
	}
}

void modifySemaphore(int x, int id_sem){
	struct sembuf operation;
	operation.sem_num = 0;
	operation.sem_op = x;
	operation.sem_flg = 0;
	semop(id_sem, &operation, 1); // semop() performs operations on selected semaphores
} // -->Despues sacar esto de aca!!

/* int semop(int semid, struct sembuf *sops, unsigned nsops);

Each of the nsops elements in the array pointed to by sops specifies an operation to be performed on a single semaphore. 
The elements of this structure are of type struct sembuf, containing the following members:
unsigned short sem_num;   semaphore number 
short          sem_op;   semaphore operation 
short          sem_flg;  operation flags 

If sem_op is a positive integer, the operation adds this value to the semaphore value (semval).

*/