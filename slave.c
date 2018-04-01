#include "include/slave.h"

int 
main(int args, char * argv[]){
	char msg[MAX_FILENAME], resultHashes[ORDERS_NUM * MSG_LENGTH];
  	char * filename, * curr = msg;
  	int running = 1;
  	queue_o orderQueue = newQueue();
  	
  	while(running){
		while((*curr = getchar()) && (*curr != NUL) && (*curr != END_CHARACTER)){
				if(*curr == VERTICAL_SLASH){
					*curr = NUL;
					filename = malloc(strlen(msg) + 1);
					strcpy(filename, msg);
					order_o order;
					order.filename = filename;
					order.processed = false;
					enQueue(orderQueue, order);
					memset(msg, 0, sizeof(msg));
					curr = msg;
				} else {
					curr++;
				}
			}

			if(*curr == END_CHARACTER) 
				break;

			processOrderQueue(orderQueue, resultHashes);
  	}

	return 0;
}

void processOrderQueue(queue_o orderQueue, char * resultHashes){
	int sizeQueue = orderQueue->size;
	int i;
		
	for(i = 0; i < sizeQueue; i++){
			node_o * temp = deQueue(orderQueue);
			processOneOrder(temp->order.filename,resultHashes);
  	}

  	sendResults(resultHashes);
}

void processOneOrder(char * filename, char * resultHashes){
  	char md5[MD5_LENGTH], temporal[MSG_LENGTH];
	int pipefd[2], pid;

  	pipe(pipefd);
  	
  	pid = fork();
	
	switch(pid){
		case -1: 
			perror("[ERROR!] Couldn't fork correctly!\n");
			exit(EXIT_FAILURE);
			break;
		case 0:  
			calculateMD5Hashes(pipefd, filename); 
			break;
		default:  
			readResults(pipefd, md5); 
			break;
	}

	sprintf(temporal,"<%s>: <%s>\n",filename, md5);
	strcat(resultHashes,temporal);
}

void calculateMD5Hashes(int * pipefd, char * filename){
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);
	execlp(MD5, MD5 ,filename, NULL);
	close(pipefd[1]);
}

void readResults(int * pipefd, char * md5){
	read(pipefd[0], md5, MD5_LENGTH);
    wait(NULL); 
    md5[MD5_LENGTH - 1] = NUL;
}

void sendResults(char * resultHashes){
	write(STDOUT_FILENO,resultHashes,strlen(resultHashes));
  	memset(resultHashes, 0, sizeof(resultHashes));
}
