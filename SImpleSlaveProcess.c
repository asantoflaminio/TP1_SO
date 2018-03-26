#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MD5_SIZE 256

void calculateHash(char *, int);

int main(int args, char * argv[]){
	char md5sum[MD5_SIZE];
	calculateHash(md5sum, MD5_SIZE);
	printf("Command ls: %s\n", md5sum);
}

void calculateHash(char * buffer, int size){
	FILE * command = popen("md5sum slaveProcess.c", "r");
	while (fgets(buffer, size, command) != 0){

	}
	pclose(command);
}