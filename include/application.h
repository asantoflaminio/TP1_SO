#ifndef APPLICATION_H
#define APPLICATION_H

#include "queue.h"
#include "slave.h"

#define ORDERS_NUM 2
#define SLAVES_NUM 2
#define SEPARATOR "/"
#define VERTICAL_SLASH "|"
#define SLAVE_EXEC "./slave"

/* Searches all available files from dirname and enqueues 
them into queue. Returns how many files where enqueued. */
int loadFiles(const char *dirname, queue_o queue, int files);

/* Start the execution of the program, once the entry 
through the command line was syntactically correct. */
void start(const char *dirname);

slaves_o * createSlaves();
queue_o assignWork(slaves_o * slaves, queue_o orderQueue, int queueSize, int * assignedOrder);

#endif
