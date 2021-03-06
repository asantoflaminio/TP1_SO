#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include "include/application.h"
#include "include/mySemaphore.h"


int 
main(int argc, char* argv[]){
	int pid, status;

	if ( ((argc != 3 || ((strcmp(argv[1], "hash") != 0)))  && ((argc != 2 || ((strcmp(argv[1], "test") != 0)))) )) {
		printf("The arguments were wrong. Correct format: " ANSI_GREEN "hash <directory>\n" ANSI_RESET);
		return -1;
	}

	if((strcmp(argv[1], "test") == 0))
		startTest();
	else{
		printf("Starting...\n");
		sleep(1.5);
		start(argv[2]);


		while ((pid=waitpid(-1,&status,0)) != -1) {
	        printf("Process %d finished\n",pid);
	    }

	    printf("Finishing application process...\n");
    }
			
	return 0;
}

void start(const char *dirname){
	int files = 0;
	int queueSize;
	int slavesQuantity;
	queue_o orderQueue;
	slaves_o * slaves;
	char **hashes;

	key_t key;
	
	int id_shmem;
	char * shm; 
	
	int id_sem;

	key = generateKey(getpid());

	printf("Creating shared memory segment...\n");
	shm = createSharedMemorySegment(&id_shmem, key);
	sleep(1.5);

	printf("Creating semaphore...\n");
	createSemaphore(&id_sem, key);
	changePermissions(id_sem);
	sleep(1.5);

   	modifySemaphore(1, id_sem);
   	menu();

   	printf("Starting application process...Press ENTER to continue!\n");
	while(getchar()!='\n');

	printf("Creating order queue...\n");
	orderQueue = newQueue();
	sleep(1.5);

	printf("Fetching files...\n");
	files = loadFiles(dirname, orderQueue, files);
	printf("%d files were fetched!\n", files);
	sleep(1.5);

	slavesQuantity = files/(2*ORDERS_NUM);

	if(files == 0){
		printf("No files to process\n");
		exit(EXIT_SUCCESS);
	}

	queueSize = orderQueue->size;

	printf("Creating %d slaves...\n", slavesQuantity);
	slaves = createSlaves(slavesQuantity);
	sleep(1.5);

	hashes = (char **)malloc(files * sizeof (char *));
	
	hashes = startProcessing(queueSize, orderQueue, slaves, hashes, shm, id_sem, slavesQuantity);
	
	modifySemaphore(1,id_sem);
	strcat(shm,"?"); 
	modifySemaphore(-1,id_sem);

	printf("Stopping slaves from working...\n");
	stopSlaves(slaves, slavesQuantity);
	sleep(1.5);

	printf("Writing results into my_hashes.txt...\n");
	writeResultIntoFile(queueSize, hashes);
	
	printf("Detaching and removing a shared memory segment...\n");
    detachAndRemoveSharedMem(id_shmem, shm);
   
   	printf("Removing semaphore...\n");
    removeSemaphore(id_sem);
    
   	freeMemory(hashes, slaves, orderQueue, files);
}

key_t generateKey(int num){
	key_t key = ftok("/home", num);
	
	if (key == -1) {
		perror(ANSI_RED"[ERROR!] " ANSI_RESET "Couldn't generate the key!");
		exit(1);
	}

	return key;
}


int loadFiles(const char *dirname, queue_o queue, int files){
	DIR *dir;
	struct dirent *dp;
	char * current;
	int length;
	dir = opendir(dirname);
	
	if( dir != NULL){
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


slaves_o * createSlaves(int slavesQuantity){
	int i;
	slaves_o * slaves;
	int flags;
	slaves = (slaves_o *)calloc(slavesQuantity, sizeof(slaves_o));

	if(slaves == NULL) {
	    perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't allocate space for slaves!\n");
	    wait(NULL);
	    _exit(1);
  	}


	for(i = 0; i < slavesQuantity; i++){
		pid_t pid;

		slaves[i].isWorking = false;

	  	if(pipe(slaves[i].pipeFatherToChild) == -1){
	        printf(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't open the pipe Father->Child!\n");
	    }

	    if(pipe(slaves[i].pipeChildToFather) == -1){
	        printf(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't open the pipe Child->Father!\n");
	    }

		pid = fork();
		flags = fcntl(slaves[i].pipeChildToFather[0], F_GETFL, 0);
		
		switch(pid){
			case -1:
				perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't fork correctly!\n");
				wait(NULL);
				exit(EXIT_FAILURE);
				break;
			case 0:
				dup2(slaves[i].pipeFatherToChild[0], STDIN_FILENO);
				if(dup2(slaves[i].pipeChildToFather[1], STDOUT_FILENO) == -1)
					perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't redirect stdout of child\n");
		 		close(slaves[i].pipeFatherToChild[1]);
		 		close(slaves[i].pipeChildToFather[0]);
				execlp(SLAVE_EXEC, SLAVE_EXEC, NULL);
				perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't execute worker in forked child!\n");
				wait(NULL);
				break;
			default:
				fcntl(slaves[i].pipeChildToFather[0], F_SETFL, flags | O_NONBLOCK);
				close(slaves[i].pipeFatherToChild[0]);
				close(slaves[i].pipeChildToFather[1]);
				break;
		}
	}

	return slaves;
}

char ** startProcessing(int queueSize, queue_o orderQueue, slaves_o * slaves, char ** hashes, char * shm, int id_sem, int slavesQuantity){
	int assignedOrder = 0;
	int finishOrder = 0;
	int pointer = 0;

	char * buff = malloc(MAX_HASH_LENGTH * sizeof(char));
	char * curr = buff;
	int i;
	
	memset(buff, 0, sizeof(MAX_HASH_LENGTH));
	while(finishOrder != queueSize){ 
		if(assignedOrder != queueSize)
			orderQueue = assignWork(slaves, orderQueue, queueSize, &assignedOrder, slavesQuantity); 	

		for(i = 0; i < slavesQuantity; i++){
			if(slaves[i].isWorking){
				while(read(slaves[i].pipeChildToFather[0], curr, 1) == 1){
					if(*curr == '\n'){
						*curr = '\0';
						slaves[i].isWorking = false;
						finishOrder++;
						hashes[pointer] = malloc(MAX_HASH_LENGTH * sizeof(char));
						memset(hashes[pointer], 0, sizeof(MAX_HASH_LENGTH));
						strcpy(hashes[pointer], buff); 
						modifySemaphore(-1,id_sem);
						strcat(shm, buff);
						strcat(shm, VERTICAL_SLASH); 
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

	free(buff);
	return hashes;
}

queue_o assignWork(slaves_o * slaves, queue_o orderQueue, int queueSize, int * assignedOrder, int slavesQuantity){
	int i, j;
	node_o * temp;

	for(i = 0; i < slavesQuantity && queueSize != *assignedOrder; i++){	
		if(slaves[i].isWorking == false){
			for(j = 0; j < ORDERS_NUM && (*assignedOrder != queueSize) ; j++){
				if(orderQueue->first->order.processed == false){
					write(slaves[i].pipeFatherToChild[1], orderQueue->first->order.filename, strlen(orderQueue->first->order.filename));
					write(slaves[i].pipeFatherToChild[1], "|", 1);
					temp = deQueue(orderQueue);
					printf("Sending %s to slave number %d\n", temp->order.filename, i);
					free(temp->order.filename);
					free(temp);
					slaves[i].isWorking = true;
					(*assignedOrder)++;
				} else {
					printf(ANSI_RED "[ERROR!] " ANSI_RESET "Trying to process a file that has already been processed!\n");
					break; 
				}

			} write(slaves[i].pipeFatherToChild[1],"", 1);
		}
	}

	return orderQueue;
}


void stopSlaves(slaves_o * slaves, int slavesQuantity){
	int i;
	for(i = 0; i < slavesQuantity; i++){
		write(slaves[i].pipeFatherToChild[1],STOP_SLAVES,1);
	}
}

void menu(){
	char input;
	
	printf(ANSI_BLUE"--- MENU ---\n"ANSI_RESET);
	printf("Please, choose one option: \n");
   	printf("1 - Show instructions to start view process\n");
   	printf("2 - Continue to application process normally. Remember: <pid>=%d\n", getpid());
   	
   	input = getOption();

   	if(input == '1')
   		manual();
}

char getOption(){
	char input, enter;
	
	printf("Option selected: ");
   	input = getchar();
   	enter = getchar();

   	if(enter != '\n' || (input != '1' && input != '2')){
   		printf(ANSI_RED "[ERROR!] " ANSI_RESET "Please choose between options given above\n");
   		cleanBuffer();
   		getOption();
   	}
   		
   	return input;			
}

void cleanBuffer(){
	while(getchar() != '\n');
}

void manual(){
	printf(ANSI_BLUE"--- MANUAL ---\n"ANSI_RESET);
   	printf("1 - Open another terminal window where your proyect is\n");
   	printf("2 - Enter the next command: " ANSI_GREEN "./view <pid>." ANSI_RESET " Remember: <pid>=%d\n", getpid());
   	printf("3 - Then continue application by following the next step\n");
   	sleep(2);
}

void writeResultIntoFile(int queueSize, char ** hashes){
	int i = 0;
	FILE * file = fopen("my_hashes.txt", "w+");
	
	if (file == NULL) {
		perror("[ERROR!] File error!\n");
		exit(1);
	}

	for(i = 0; i < queueSize ; i++){
		fputs(hashes[i],file);
		fputs("\n",file);
	}

	fclose(file);
}

void freeMemory(char ** hashes, slaves_o * slaves, queue_o orderQueue, int files){
	int j;

	for(j = 0; j < files; j++){
		free(hashes[j]);
	}

	free(hashes);
	free(slaves);
	free(orderQueue);
}
