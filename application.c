#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "include/application.h"
#include "include/queue.h"
#include "include/order.h"
#include "include/types.h"
#include "include/slave.h"


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
	int files = 0;
	int i;
	queue_o orderQueue;
	slaves_o * slaves;

	//Create queue
	printf("Creating order queue...\n");
	orderQueue = newQueue();

	//Fetch files
	printf("Fetching files...\n");
	files = loadFiles(dirname, orderQueue, files);
	printf("... %d files were fetched!\n", files);
	
	if(files == 0){
		printf("%s\n", "No files to process");
		exit(EXIT_SUCCESS);
	} else {
		/* Probando que se guarde todo ok en la queue - Despues borrar esto */
		node_o * current = orderQueue->first;
		
		while(current != NULL){
			printf("processed?: %d - ", current->order.processed);
			printf("filename: %s\n", current->order.filename);
			current = current->next;
		}
		printf("Queue size: %d\n", orderQueue->size);
	}
	
	
	//Create slaves
	slaves = createSlaves();
	printf("Despues de crear los slaves\n");
	//Assign them what to do
	int assignedOrder = 0;
	int finishOrder = 0;
	int queueSize = orderQueue->size;
	//Esperamos a que los hijos finalizen de procesar los md5
	//wait(NULL); 

	char hashes[files][100];
	int pointer = 0;

	char * curr = hashes[pointer];
	
	while(assignedOrder != queueSize){ //Deberia ser assignedOrder != queueSize, solo para probar
		
		orderQueue = assignWork(slaves, orderQueue, queueSize, &assignedOrder); 
		//printf("--Despues de asignar %d tareas. Quedan %d--\n", assignedOrder, queueSize - assignedOrder);	


		for(i = 0; i < SLAVES_NUM; i++){
			if(slaves[i].isWorking){
				while(read(slaves[i].pipeChildToFather[0], curr, 1) == -1);	
				printf("Encontre un -1\n");
				while(read(slaves[i].pipeChildToFather[0], curr, 1) == 1){
					if(*curr == '\n'){
						*curr = '\0';
						pointer++;
						slaves[i].isWorking = false;
						curr = hashes[pointer];
					}
					else{
						curr++;
					}
				}
			}
		}
	
		if(assignedOrder == queueSize){ //Deberia ser assignedOrder == queueSize, solo para probar
			for(i = 0; i < SLAVES_NUM; i++){
					write(slaves[i].pipeFatherToChild[1],"x",1);
			}

		} 
	}

	int j = 0;
	for(j = 0; j < queueSize ; j++)
		printf("Desde Padre: %s\n" , hashes[j]);
	//Imprimo para revisar su correcto procesamiento


	int pid2;
	int status;
	while ((pid2=waitpid(-1,&status,0))!=-1) {
        printf("Process %d finished\n",pid2);
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
				
				if(dp->d_type == DT_DIR){
					//printf("Opening directory... %s\n", dp->d_name);
					files = loadFiles(current, queue, files);
				} else {
					//printf("Opening file %s\n", dp->d_name);
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
	    perror("[ERROR!] Couldn't allocate space for slaves!");
	    wait(NULL);
	    _exit(1);
  	}


	for(i = 0; i < SLAVES_NUM; i++){
		/* Comunicación bidireccional: Comunico a mi aplicacion via dos pipe con uno de los esclavos */
		pid_t pid;

		slaves[i].isWorking = false;

	  	if( pipe(slaves[i].pipeFatherToChild) == -1 ) {
	        printf("[ERROR!] Couldn't open the pipe Father->Child!\n");
	    }

	    if( pipe(slaves[i].pipeChildToFather) == -1 ) {
	        printf("[ERROR!] Couldn't open the pipe Child->Father!\n");
	    }


	    
	   	//fcntl(slaves[i].pipeChildToFather[0], F_SETFL,  O_NONBLOCK);

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
					perror("[ERROR!] Couldn't redirect stdout of child");
		 		close(slaves[i].pipeFatherToChild[1]);
		 		close(slaves[i].pipeChildToFather[0]);
				execlp(SLAVE_EXEC, SLAVE_EXEC, NULL);
				perror("[ERROR!] Couldn't execute worker in forked child!");
				wait(NULL);
				break;
			default:
				//If i'm the application process
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
	printf("Envie pedidos\n");
	for(i = 0; i < SLAVES_NUM && queueSize != *assignedOrder; i++){	
		if(slaves[i].isWorking == false){
			for(j = 0; j < ORDERS_NUM; j++){
				if(orderQueue->first->order.processed == false){
					write(slaves[i].pipeFatherToChild[1], orderQueue->first->order.filename, strlen(orderQueue->first->order.filename));
					write(slaves[i].pipeFatherToChild[1], "|", 1);
					node_o * temp = deQueue(orderQueue);
					printf("Enviado: %s\n", temp->order.filename);
					slaves[i].isWorking = true;
					(*assignedOrder)++;
				} else {
					printf("[ERROR!] Trying to process a file that has already been processed !\n");
					break; //SACAR
				}

			} write(slaves[i].pipeFatherToChild[1], "", 1);
		}
	}

	return orderQueue;
}
