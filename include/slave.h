#ifndef SLAVE_H
#define SLAVE_H

#include "order.h"
#include "queue.h"

/* Character send by application to end slave process. */
#define END_CHARACTER '+'
/* Filename lenght limit for our program. */
#define MAX_FILENAME 100
/* Command to calculate hashes */
#define MD5 "md5sum"
/* 32 characters of MD5 + end of string. */
#define MD5_LENGTH 33
/* Slave's answer length: 100 of MAX_FILENAME + 33 of MD5_LENGTH + a bit of aux space. */
#define MSG_LENGTH 140
/* This character is a control character with the value zero. */
#define NUL '\0'
/* Number of orders the application will send per slave. */
#define ORDERS_NUM 2
/* Represents the separation between two different orders. */
#define VERTICAL_SLASH '|'

/* Struct for each slave that contains two pipes to communicate bidireccionaly with 
the application and a boolean value that indicates if it's currently processing something. */
typedef struct {
	int pipeFatherToChild[2];
	int pipeChildToFather[2];
	boolean isWorking;
} slaves_o;

/* Receives a queue of tasks and start processing them separately. */
void processOrderQueue(queue_o orderQueue, char * resultHashes);

/* Creates a pipe and forks the process in order to redirect 
stdout of md5sum to the program. */
void processOneOrder(char * filename, char * resultHashes);

/* Calculates MD5 hashes for the file given and redirects 
what is written in stdout to the child's writing pipe. */
void calculateMD5Hashes(int * pipefd, char * filename);

/* Reads from the father reading pipe the hash
MD5 and save the result in md5. */
void readResults(int * pipefd, char * md5);

/* Sends the M5D hashes to the application process. */
void sendResults(char * resultHashes);

#endif
