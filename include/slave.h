#ifndef SLAVE_H
#define SLAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#include "order.h"
#include "queue.h"

/* Character send by application to end slave process. */
#define STOP_SLAVES_CHARACTER '+'
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
#define VERTICAL_SLASH_CHARACTER '|'
/* Represents the max length path of a file. */
#define MAX_LENGHT_FILES 60
/* Represents the max number of files to process. */
#define MAX_FILES_TO_PROCESS 3
/* Represents the max number of characters of all the hashes of files processed. */
#define MAX_LENGHT_HASHES 2024
/* Represents the max number of characters per hash of a single file. */
#define MAX_LENGHT_HASH 100
/* Represents the option selected to quit menu. */
#define EXIT 3

/* Struct for each slave that contains two pipes to communicate bidireccionaly with 
the application and a boolean value that indicates if it's currently processing something. */
typedef struct {
	int pipeFatherToChild[2];
	int pipeChildToFather[2];
	boolean isWorking;
} slaves_o;

/* Receives a queue of tasks and start processing them separately. */
void processOrderQueue(queue_o, char *);

/* Creates a pipe and forks the process in order to redirect 
stdout of md5sum to the program. */
void processOneOrder(char *, char *);

/* Calculates MD5 hashes for the file given and redirects 
what is written in stdout to the child's writing pipe. */
void calculateMD5Hashes(int *, char *);

/* Reads from the father reading pipe the hash
MD5 and save the result in md5. */
void readResults(int *, char *);

/* Sends the M5D hashes to the application process. */
void sendResults(char *);

/* ---------------- Test cases functions ---------------- */

/* Test function for bidirectional communication. */
void communicationTestFunction();

/* Starts the test functions for slave.c. */
void slaveTest();

/* Test for an amount of files. */
void testCalculateHashesOfDirectory();

/* Test for a single hash file. */
void testCalculateHashOfFile();

/* Takes the path of a file. */
void givenFileToProcess(char *);

/* Test the path of an amount of files. */
void givenDirectoryToProcess(char *, queue_o, queue_o, int *);

/* Calls processOneOrder function. */
void whenProcessingFile(char *, char *);

/* Calls processOrderQueue function. */
void whenProcessingFiles(queue_o, char *);

/* Calculates one hash by calling md5sum after forking. */
void thenHashWasCalculated(const char *, char *);

/* Calculates hashes hash by calling md5sum after forking. */
void thenHashesWereCalculated(int, queue_o);

#endif
