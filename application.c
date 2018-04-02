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
#include "include/mysemaphore.h"


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
	queue_o orderQueue;
	slaves_o * slaves;
	
	key_t key;
	
	int id_shmem;
	char *shm; 
	
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

	if(files == 0){
		printf("No files to process\n");
		exit(EXIT_SUCCESS);
	}

	printf("Creating %d slaves..\n", SLAVES_NUM);
	slaves = createSlaves();
	sleep(1.5);

	/* Mi idea seria meter todo este choclo en una funcion pero hasta no ver de cambiar lo de 
	char ** de la shared memory mejor lo dejo aca */
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
						strcat(shm, buff);
						strcat(shm, "|"); 
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
	
	modifySemaphore(1,id_sem);
	strcat(shm,"?"); //-->Caracter donde finaliza mi string largo compartido para reconocer q tengo que salir del while(1) en view.. CAMBIAR
	modifySemaphore(-1,id_sem);
	/* Aca termina el choclo */


	printf("Stopping slaves from working..\n");
	stopSlaves(slaves);
	sleep(1.5);

	writeResultIntoFile(queueSize, hashes);
	
    detachAndRemoveSharedMem(id_shmem, shm);
   
    removeSemaphore(id_sem);
    
    int j;
    for(j = 0; j < files; j++){
		free(hashes[j]);
	}

	free(hashes);

	free(slaves);
	free(orderQueue);
    
}

key_t generateKey(int num){
	key_t key = ftok("/home", num);
	
	if (key == -1) {
		perror(ANSI_RED"[ERROR!] " ANSI_RESET "Couldn't generate the key!");
		exit(1);
	}

	return key;
}

char * createSharedMemorySegment(int * id_shmem, key_t key){
	char * shm;

	*id_shmem = shmget(key, MYSIZE, 0777 | IPC_CREAT);
	if(*id_shmem < 0){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't create shared memory!");
		exit(1);
	}
	
	shm = (char*) shmat(*id_shmem, 0, 0);
	if(shm == (char*) -1){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't take the shared segment!\n");
		exit(1);
	}

	return shm;
}

void createSemaphore(int * id_sem, key_t key){
	*id_sem = semget (key, 1, 0666 | IPC_CREAT);
	if (*id_sem == -1){
		perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't create semaphore!\n");
		exit (1);
	}
}

void changePermissions(int id_sem){
	union semun arg;

	arg.val = 0;
	semctl(id_sem, 0, SETVAL, &arg);
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


slaves_o * createSlaves(){
	int i;
	slaves_o * slaves;

	slaves = (slaves_o *)calloc(SLAVES_NUM, sizeof(slaves_o));

	if(slaves == NULL) {
	    perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't allocate space for slaves!\n");
	    wait(NULL);
	    _exit(1);
  	}


	for(i = 0; i < SLAVES_NUM; i++){
		pid_t pid;

		slaves[i].isWorking = false;

	  	if(pipe(slaves[i].pipeFatherToChild) == -1){
	        printf(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't open the pipe Father->Child!\n");
	    }

	    if(pipe(slaves[i].pipeChildToFather) == -1){
	        printf(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't open the pipe Child->Father!\n");
	    }

		//Start slave
		pid = fork();
		int flags = fcntl(slaves[i].pipeChildToFather[0], F_GETFL, 0);
		
		switch(pid){
			case -1:
				perror(ANSI_RED "[ERROR!] " ANSI_RESET "Couldn't fork correctly!\n");
				wait(NULL);
				exit(EXIT_FAILURE);
				break;
			case 0:
				//If i'm the child, execute ./slave
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
					free(temp->order.filename);
					free(temp);
					slaves[i].isWorking = true;
					(*assignedOrder)++;
				} else {
					printf(ANSI_RED "[ERROR!] " ANSI_RESET "Trying to process a file that has already been processed!\n");
					break; //SACAR
				}

			} write(slaves[i].pipeFatherToChild[1],"", 1);
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

void menu(){
	printf(ANSI_BLUE"--- MENU ---\n"ANSI_RESET);
	printf("Please, choose one option: \n");
   	printf("1 - Show instructions to start view process\n");
   	printf("2 - Continue to application process normally. Remember: <pid>=%d\n", getpid());
   	
   	char input = getOption();

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

void detachAndRemoveSharedMem(int id_shmem, char * shm){
	shmdt(shm);
	shmctl(id_shmem, IPC_RMID, 0);
}

void removeSemaphore(int id_sem){
	semctl(id_sem, 0, IPC_RMID); 
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
