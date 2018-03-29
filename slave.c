#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "include/slave.h"
#include "include/order.h"
#include "include/queue.h"

int 
main(int args, char * argv[]){
	char msg[MAX_FILENAME];
  	char * curr = msg;
  	char * file;
  	queue_o orderQueue = newQueue();
  	int sizeQueue;
  	int i;

  	//Los pasajes relativos estan separados por un "|" por lo que recoletamos la data
  	//y la vamos fraccionando, para colocarla en la cola y procesarla
	while((*curr=getchar()) && *curr!='\0'){
		if(*curr == '|'){
			*curr = '\0';
			file = malloc(strlen(msg) + 1);
			strcpy(file, msg);
			order_o order;
			order.filename = file;
			order.processed = false;
			//printf("Encolando: %s\n" , msg);
			enQueue(orderQueue, order);
			memset(msg, 0, sizeof(msg));
			curr = msg;
		}
		else{
			curr++;
		}
	}

	sizeQueue = orderQueue->size;
	for(i = 0; i < sizeQueue; i++){
		node_o * temp = deQueue(orderQueue);
		processOrder(temp->order.filename);
	}
	
	return 0;
}




void processOrder(char * filename){
  	char md5sum[MD5_SIZE] = "";
	int pipefd[2];
  	pipe(pipefd);
  	int pid;
  	pid = fork();
	switch(pid){
		case -1: 
			perror("Unable to do fork"); 
			exit(EXIT_FAILURE);
			break;
		case 0:  
			sonFunction(pipefd, filename); 
			break;
		default:  
			fatherFunction(pipefd, md5sum); 
			break;
	}

	printf("Hash MD5 of %s: %s\n",filename, md5sum);
}

void sonFunction(int * pipefd, char * filename){
	// Redirijimos lo que se escribe en stdout a la tuberia de escritura del hijo
    dup2(pipefd[1], 1);
    close(pipefd[0]);
	execlp("md5sum", "md5sum" ,filename, NULL);
	close(pipefd[1]);
	
}

void fatherFunction(int * pipefd, char * md5sum){
	read(pipefd[0], md5sum, MD5_SIZE);
    // Esperamos a que todos los hijos terminen su ejecucion, 
	// en este caso, solo esperamos al unico hijo del fork()
    wait(NULL); 
    md5sum[MD5_SIZE - 1] = '\0';
    close(pipefd[0]);
    close(pipefd[1]);
}
