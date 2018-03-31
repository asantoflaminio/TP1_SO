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

int 
main(int argc, char* argv[]){
	
	if (argc != 3 || (strcmp(argv[1], "hash") != 0)) {
		printf("The arguments were wrong. Format: hash <directory>\n");
		return -1;
	}

	start(argv[2]);
	return 0;
}

void start(const char *dirname){
	int files;
	queue_o orderQueue;
	slaves_o * slaves;
	FILE *file;

	key_t key;
	int id;
	int idSem;
	char *shm; 

	key = ftok("/bin/ls", 123); 
	if (key == -1) {
		perror("[ERROR!] Couldn't generate the key!\n");
		exit(1);
	}

	id = shmget(key, MYSIZE, 0777 | IPC_CREAT);
	if(id < 0){
		perror("[ERROR!] Couldn't create shared memory!\n");
		exit(1);
	}
	
	shm = shmat(id, 0, 0);
	if(shm == (char*) -1){
		perror("[ERROR!] Couldn't take the shared segment!\n");
		exit(1);
	}
	
	idSem = semget (key, 1, 0777 | IPC_CREAT);
	if (idSem == -1)
	{
		perror("[ERROR!] Couldn't create semaphore!\n");
		exit (1);
	}
	
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

	free(buff);

	printf("Stopping slaves from working..\n");
	stopSlaves(slaves);
	

	file = fopen("my_hashes.txt", "w+");
	if (file==NULL) {
		perror("[ERROR!] File error!\n");
		exit(1);
	}
	
	//char* myhashes = malloc(queueSize*hashes[0]);

	int j = 0;
	for(j = 0; j < queueSize ; j++){

		printf("Desde Padre: %s\n" , hashes[j]);
		fputs(hashes[j],file);
		fputs("\n",file);
		
	}

	int pid, status;
	while ((pid=waitpid(-1,&status,0)) != -1) {
        printf("Process %d finished\n",pid);
    }
    
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

