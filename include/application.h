#ifndef APPLICATION_H
#define APPLICATION_H

#include "queue.h"

#define SLAVE_EXEC "./slave"

/* Searches all available files from dirname and enqueues 
them into queue. Returns how many files where enqueued. */
int loadFiles(const char *dirname, queue_o queue, int files);

/* Start the execution of the program, once the entry 
through the command line was syntactically correct. */
void start(const char *dirname);



#endif
