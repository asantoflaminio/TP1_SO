#ifndef APPLICATIONPROCESS_H
#define APPLICATIONPROCESS_H

#include "queue.h"

/* Searches all available files from dirname and enqueues 
them into queue. Returns how many files where enqueued. */
int loadFiles(const char *dirname, queue_o queue, int files);

/* Start the execution of the program, once the entry 
through the command line was syntactically correct. */
void start(const char *dirname);

#endif