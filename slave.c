#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "include/slave.h"
#include "include/order.h"

int 
main(int args, char * argv[]){
	char msg[MAX_FILENAME];
  	char * curr = msg;
		
	while((*curr=getchar()) && *curr!='\0')
		curr++; 
	
	printf("I'm the slave and I received %s file to process\n", msg);
	processOrder(msg);
}


int processOrder(char * filename){
  	char md5sum[MD5_SIZE];
	int pipefd[2];
  	pipe(pipefd);
  	int pid;

  	pid = fork();
	switch(pid){
		case -1: perror("Unable to do fork"); break;
		case 0:  sonFunction(pipefd, filename); break;
		default:  fatherFunction(pipefd, md5sum); break;
	}

}

void sonFunction(int * pipefd, char * filename){
	dup2(pipefd[1], STDOUT_FILENO); //redirijimos lo que se escribe en stdout a la tuberia de escritura del hijo
    close(pipefd[0]);
	execlp("md5sum", "md5sum" ,filename, NULL);
	close(pipefd[1]);
}

void fatherFunction(int * pipefd, char * md5sum){
	read(pipefd[0], md5sum, MD5_SIZE);
    wait(NULL); //Esperamos a que todos los hijos terminen su ejecucion, en este caso, solo esperamos al unico hijo del fork()
    md5sum[MD5_SIZE - 1] = '\0';
    close(pipefd[0]);
    close(pipefd[1]);
}
