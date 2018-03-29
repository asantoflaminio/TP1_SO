#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "include/application.h"
#include "include/queue.h"
#include "include/order.h"
#include "include/types.h"

#define SEPARATOR "/"
#define ORDERS_NUM 4

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
	queue_o orderQueue;

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
		
		//printf("Queue size: %d\n", orderQueue->size);
	}
		
	/* Comunicación unidireccional: Comunico a mi aplicacion via un pipe con el esclavo */
	int pipeFatherToChild[2];
	int pipeChildToFather[2];
	pid_t pid;
	int i;
	int sizeQueue = orderQueue->size;
	

  	if( pipe(pipeFatherToChild) == -1 ) {
        printf("Error while opening the pipe!\n");
    }

    if( pipe(pipeChildToFather) == -1 ) {
        printf("Error while opening the second the pipe!\n");
    }

	//Start slave
	pid = fork();
	
	switch(pid){
	case -1:
		perror("Fork error\n");
		wait(NULL);
		exit(EXIT_FAILURE);
		break;
	case 0:
		//If i'm the child, execute ./slave
		dup2(pipeFatherToChild[0], STDIN_FILENO);
		dup2(pipeChildToFather[1], STDOUT_FILENO);
 		close(pipeFatherToChild[1]);
 		close(pipeChildToFather[0]);
		execlp(SLAVE_EXEC, SLAVE_EXEC, NULL);
		perror("[ERROR!] Couldn't execute worker in forked child!");
		wait(NULL);
		break;
	default:
		//If i'm the application process
		dup2(pipeChildToFather[0], STDIN_FILENO);
		close(pipeFatherToChild[0]);
		close(pipeChildToFather[1]);
		for(i = 0; i < ORDERS_NUM && i < sizeQueue; i++){
			write(pipeFatherToChild[1], orderQueue->first->order.filename, strlen(orderQueue->first->order.filename));
			write(pipeFatherToChild[1], "|", 1);
			node_o * temp = deQueue(orderQueue);
			printf("Enviado: %s\n", temp->order.filename);
		}
		write(pipeFatherToChild[1], "", 1);
		break;
	}

	wait(NULL); //Esperamos a que el hijo finalize de procesar los md5

	//En esta seccion extraemos la informacion procesada por los hijos

	//DEBERIA HACERSE CON char ** y malloc
	char hashes[4][100];
	int pointer = 0;
	int character = 0;
	char * curr = hashes[pointer];
	while(read(pipeChildToFather[0], curr, 1) == 1){
		if(*curr == '\n'){
			*curr = '\0';
			pointer++;
			curr = hashes[pointer];
		}
		else{
			curr++;
		}
	}
	//Imprimo para revisar su correcto procesamiento
	int j = 0;
	for(j = 0; j < 4 ; j++)
		printf("Desde Padre: %s\n" , hashes[j]);

	int pid2;
	int status;
	while ((pid2=waitpid(-1,&status,0))!=-1) {
        printf("Process %d terminated\n",pid2);
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