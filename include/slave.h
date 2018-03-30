#ifndef SLAVE_H
#define SLAVE_H

#include "order.h"

/* 32 characters of md5 + end of string */
#define MD5_SIZE 33
/* Filename lenght limit */
#define MAX_FILENAME 256

typedef struct {
	int pipeFatherToChild[2];
	int pipeChildToFather[2];
	boolean isWorking;
} slaves_o;

/* */
void sonFunction(int * pipefd, char * filename);

/* */
void fatherFunction(int * pipefd, char * md5sum);

/* */
void processOrder(char * filename, char * resultHashes);

#endif
