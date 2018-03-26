/*Proceso esclavo generico*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MD5_SIZE 50

void sonFunction(int *);
void fatherFunction(int *, char *);

int main(int args, char * argv[]){
	char md5sum[MD5_SIZE];
	int pipeFileDescriptor[2];
  	pipe(pipeFileDescriptor);
  	int pid;

  	pid = fork();
	switch(pid){
		case -1: perror("Unable to do fork"); break;
		case 0:  sonFunction(pipeFileDescriptor); break;
		default:  fatherFunction(pipeFileDescriptor, md5sum); break;
	}
	printf("%s", md5sum);
}

void sonFunction(int * pipeFileDescriptor){
	dup2(pipeFileDescriptor[1], 1); //redirijimos lo que se escribe en stdout a la tuberia de escritura del hijo, 
									//que iria a la de lectura del padre
    close(pipeFileDescriptor[0]);
	execlp("md5sum", "md5sum" ,"slaveProcess.c", NULL);
	close(pipeFileDescriptor[1]);
}

void fatherFunction(int * pipeFileDescriptor, char * md5sum){
	read(pipeFileDescriptor[0], md5sum, MD5_SIZE);
    wait(NULL); //Esperamos a que todos los hijos terminen su ejecucion, en este caso, solo esperamos al unico hijo del fork()
    md5sum[MD5_SIZE - 1] = '\0';
    close(pipeFileDescriptor[0]);
    close(pipeFileDescriptor[1]);
}