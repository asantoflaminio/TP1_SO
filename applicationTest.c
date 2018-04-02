#include "include/application.h"

void startTest(){
	printf("-----------TEST CASES-----------\n");
	testBidirectionalComunication();
	testRedistributionOfOrders();
}

void testBidirectionalComunication(){
	char message[MAX_LENGTH];
	char messageReturned[MAX_LENGTH];
	int pipeFatherToChild[2];
	int pipeChildToFather[2];

	givenString(message);

	whenSlaveIsExecuted(pipeFatherToChild, pipeChildToFather);
	printf("Done execution\n");
	whenStringIsSentToSlave(message, pipeFatherToChild);
	printf("Message sent\n");
	whenStringIsReturned(messageReturned, pipeChildToFather);
	thenStringIsReturned(message, messageReturned);
}

void testRedistributionOfOrders(){
	queue_o orderQueue;
	slaves_o * slaves;
	int files = 0;

	orderQueue = newQueue();
	files = loadFiles(DIRNAMETEST, orderQueue, files);
	slaves = createSlaves();
}

void givenString(char * message){
	printf("Write a message to slave: ");
	scanf("%s", message);
}

void whenSlaveIsExecuted(int * pipeFatherToChild, int * pipeChildToFather){

	int pid;

	pipe(pipeFatherToChild);
	pipe(pipeChildToFather);


	pid = fork();

	switch(pid){
		case -1: perror("There was an error creating the child");
				 wait(NULL);
				 break;
		case 0:  dup2(pipeFatherToChild[0], STDIN_FILENO);
				 close(pipeFatherToChild[1]);
				 dup2(pipeChildToFather[1], STDOUT_FILENO);
				 close(pipeChildToFather[0]);
				 execlp(SLAVE_EXEC, SLAVE_EXEC, "arg", NULL);
				 break;
		default: close(pipeFatherToChild[0]);
				 close(pipeChildToFather[1]);
				 break;	
	}
}

void whenStringIsSentToSlave(char * message, int * pipeFatherToChild){
	write(pipeFatherToChild[1], message, strlen(message) + 1);
}


void whenStringIsReturned(char * messageReturned, int * pipeChildToFather){
	char * current = messageReturned;
	while(read(pipeChildToFather[0], current, 1) == 1){
		current++;
	}
}

void thenStringIsReturned(const char * message, const char * messageReturned){
	if(! strcmp(message,messageReturned)){
		printf("Success in sending and receiving the message!!!\n");
	}else{
		printf("Failed to send and receive the message\n");
		printf("Original message: %s\n", message);
		printf("Message returned: %s\n", messageReturned);
	}

}
