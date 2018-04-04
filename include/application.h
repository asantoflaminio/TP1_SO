#ifndef APPLICATION_H
#define APPLICATION_H

#include "mySemaphore.h"
#include "mySharedMemory.h"
#include "queue.h"
#include "slave.h"
#include "order.h"
#include "queue.h"
#include "styles.h"
#include "types.h"

/* Quantity of orders that are sent together. */
#define ORDERS_NUM 2
/* The quantity of slaves that are working in our program. */
#define SLAVES_NUM 3
/* To list directories. */
#define SEPARATOR "/"
/* Character used to separate messages. */
#define VERTICAL_SLASH "|"
/* To execute slave. */
#define SLAVE_EXEC "./slave"
/* Character send by application to end slave process. */
#define STOP_SLAVES "+"
#define MAX_LENGTH 80
#define DIRNAMETEST "test"

/* Start the execution of the program, once the entry 
through the command line was syntactically correct. */
void start(const char *);

/* Generates an IPC key. */
key_t generateKey(int);

/* Searches all available files from dirname and enqueues 
them into queue. Returns how many files where enqueued. */
int loadFiles(const char *, queue_o , int);

/* Creates slaves and saved them in a vector of pointers to the struct slaves_o. 
Generates the connection betweeen application and slaves if it is necessary. */
slaves_o * createSlaves();

/* Cycles assigning work to slaves and reading from their pipes until there are no more files to process */
char ** startProcessing(int queueSize, queue_o orderQueue, slaves_o * slaves, char ** hashes, char * shm, int id_sem);

/* Assigns orders to slaves only if they are currently not working. 
If an order was assigned, dequeues it. */
queue_o assignWork(slaves_o *, queue_o, int, int *);

/* Stops slave execution by writing them a special character. */
void stopSlaves(slaves_o *);

/* Displays a menu. */
void menu();

/* Gets the option of the menu selected by the user. */
char getOption();

/* Displays a manual of how to start view process. */
void manual();

/* Cleans buffer. Useful if the input of the user was wrong. */
void cleanBuffer();

/* Opens a file and writes there the results saved in hashes. Then closes it. */
void writeResultIntoFile(int, char **);


/* ---------------- Test cases functions ---------------- */

void startTest();

void testBidirectionalComunication();

void testRedistributionOfOrders();

void givenString(char *);

void whenSlaveIsExecuted(int *, int *);

void whenStringIsSentToSlave(char *, int *);

void whenStringIsReturned(char *, int *);

void thenStringIsReturned(const char *, const char *);

#endif
