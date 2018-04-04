#include "include/slave.h"
#include "include/application.h"

int 
main(int args, char * argv[]){
	char msg[MAX_FILENAME], resultHashes[ORDERS_NUM * MSG_LENGTH];
  	char * file, * curr = msg;
  	int running = 1;
  	queue_o orderQueue = newQueue();

  	if(args != 1){
  		free(orderQueue);
  		if(strcmp(argv[1],"test") == 0)
  			slaveTest();
  		else
  			communitacionTestFunction();
  		return 0;
  	}
  	


  	while(running){
		while((*curr = getchar()) && (*curr != NUL) && (*curr != END_CHARACTER)){
				if(*curr == VERTICAL_SLASH){
					*curr = NUL;
					file = malloc(strlen(msg) + 1);
					strcpy(file, msg);
					order_o order;
					order.filename = file;
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

  	free(orderQueue);

	return 0;
}

void processOrderQueue(queue_o orderQueue, char * resultHashes){
	int sizeQueue = orderQueue->size;
	int i;
		
	for(i = 0; i < sizeQueue; i++){
			node_o * temp = deQueue(orderQueue);
			processOneOrder(temp->order.filename,resultHashes);
			free(temp->order.filename);
			free(temp);
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



void slaveTest(){
	int optionSelected;

	do{
		printf("Select an option from 1 to 3 to start a test\n1 - Process a single hash\n2 - Process all hashes from directory\n");
		printf("3 - Exit\n\n");
		printf("Option selected: ");
		scanf("%d", &optionSelected);
		printf("\n");
		switch(optionSelected){
			case 1: testCalculateHashOfFile();break;
			case 2: testCalculateHashesOfDirectory();break;
			default: break;
		}
	}while(optionSelected != EXIT);


}

void testCalculateHashOfFile(){
	char file[MAX_LENGHT_FILES]; 
	char resultHash[MAX_LENGHT_HASH];
	memset(resultHash, 0, sizeof(resultHash));
	givenFileToProcess(file);
	whenProcessingFile(file, resultHash);
	thenHashWasCalculated(resultHash, file);
}

void testCalculateHashesOfDirectory(){
	char directory[MAX_LENGHT_FILES];
	char resultHashes[MAX_LENGHT_HASHES];
	memset(resultHashes, 0, sizeof(resultHashes));
	int filesToProcess = 0;
	queue_o queue = newQueue();
	queue_o realQueue = newQueue();

	givenDirectoryToProcess(directory,queue,realQueue,&filesToProcess);

	whenProcessingFiles(queue, resultHashes);

	thenHashesWereCalculated(filesToProcess,realQueue);

	free(queue);
	free(realQueue);

}


void givenFileToProcess(char * file){
	printf("Write the path of the file: ");
	scanf("%s", file);
	printf("\n");
}

void givenDirectoryToProcess(char * directory,queue_o queue,queue_o realQueue, int * filesToProcess){
	printf("Enter a directory name: ");
	scanf("%s", directory);
	*filesToProcess = loadFiles(directory,queue,*filesToProcess);
	loadFiles(directory,realQueue,*filesToProcess);
}

void whenProcessingFile(char * file, char * resultHash){
	processOneOrder(file, resultHash);
}

void whenProcessingFiles(queue_o queue, char * resultHashes){
	printf("All the hashes calculated by slaves are: \n");
	processOrderQueue(queue, resultHashes);
	printf("\n");
}

void thenHashWasCalculated(const char * resultHash, char * filename){
	printf("The hash calculated was : %s", resultHash);
	switch(fork()){
		case -1: printf("Failure: Unable tu fork\n");break;
		case 0: execlp(MD5, MD5 ,filename, NULL);break;
		default: wait(NULL); printf("If both hashes are equal, then the test is succesful\n\n");break;
	}
	
}

void thenHashesWereCalculated(int filesToProcess, queue_o realQueue){
	int i;
	char filename[MAX_FILENAME];
	printf("The real hashes of files are: \n");
	for(i = 0; i < filesToProcess; i++){
		node_o * temp = deQueue(realQueue);
		strcpy(filename,temp->order.filename);
		switch(fork()){
			case -1: printf("Failure: Unable tu fork\n");break;
			case 0: execlp(MD5, MD5 ,filename, NULL);break;
			default: wait(NULL);break;
		}
		free(temp->order.filename);
		free(temp);
	}
	printf("\nIf all hashes are equal, then the test is succesful\n\n");
}

void communitacionTestFunction(){
	char message[MAX_FILENAME];
	memset(message, 0, sizeof(message));
	char * current = message;
	while((*current = getchar()) && (*current != NUL)){
		current++;
	}
	
	write(1,message,strlen(message) + 1);
	return;
}

int loadFiles(const char *dirname, queue_o queue, int files){
	DIR *dir;
	struct dirent *dp;
	char * current;
	int length;
	dir = opendir(dirname);
	
	if(dir != NULL){
		while((dp = readdir(dir)) != NULL){
			if((strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name,"..")!=0)){

				length = strlen(dirname) + strlen(dp->d_name) + strlen(SEPARATOR) + 1;
				
				if(length > MAX_FILENAME){
					 perror(ANSI_RED "[ERROR!] " ANSI_RESET "One file exceeded our filename limit!\n");
	    			_exit(1);
				}

				current = malloc(length);
				strcat(strcat(strcpy(current, dirname), SEPARATOR), dp->d_name);

				if(dp->d_type == DT_DIR){
					files = loadFiles(current, queue, files);
					free(current);
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
