#ifndef APPLICATION_H
#define APPLICATION_H

#include "queue.h"
#include "slave.h"

#define MYSIZE 1000 //no se por ahora le puse este size tmbn definido en view.h
#define ORDERS_NUM 2
#define SLAVES_NUM 3
#define SEPARATOR "/"
#define VERTICAL_SLASH '|'
#define SLAVE_EXEC "./slave"
#define VIEW_EXEC "./view"
#define MAX_LENGTH 50
#define DIRNAMETEST "test"
/* Character send by application to end slave process */
#define STOP_SLAVES "+"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN    "\x1b[32m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_RESET   "\x1b[0m"



/* Searches all available files from dirname and enqueues 
them into queue. Returns how many files where enqueued. */
int loadFiles(const char *dirname, queue_o queue, int files);

/* Start the execution of the program, once the entry 
through the command line was syntactically correct. */
void start(const char *dirname);

slaves_o * createSlaves();

queue_o assignWork(slaves_o * slaves, queue_o orderQueue, int queueSize, int * assignedOrder);

void stopSlaves(slaves_o * slaves);

void menu();

void manual();

char getOption();

void cleanBuffer();

void detachAndRemoveSharedMem(int id_shmem, char * shm); // --> La idea es sacar esto de aca y hacer un .c propio para shared mem

void removeSemaphore(int id_sem); // --> La idea es sacar esto de aca y hacer un .c propio para semaforos

key_t generateKey(int num);

char * createSharedMemorySegment(int * id_shmem, key_t key); // --> La idea es sacar esto de aca y hacer un .c propio para shared mem

void createSemaphore(int * id_sem, key_t key); // --> La idea es sacar esto de aca y hacer un .c propio para semaforos

void writeResultIntoFile(int queueSize, char ** hashes);

void changePermissions(int id_sem); // --> La idea es sacar esto de aca y hacer un .c propio para semaforos

/*Test cases functions*/

void startTest();

void testBidirectionalComunication();

void testRedistributionOfOrders();

void givenString(char * message);

void whenSlaveIsExecuted(int * pipeFatherToChild, int * pipeChildToFather);

void whenStringIsSentToSlave(char * message, int * pipeFatherToChild);

void whenStringIsReturned(char * messageReturned, int * pipeChildToFather);

void thenStringIsReturned(const char * message, const char * messageReturned);

#endif
