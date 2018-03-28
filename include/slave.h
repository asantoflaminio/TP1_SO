#ifndef SLAVE_H
#define SLAVE_H

#include "order.h"

#define MD5_SIZE 34
#define MAX_FILENAME 256

/* */
void sonFunction(int * pipefd, char * filename);

/* */
void fatherFunction(int * pipefd, char * md5sum);

int processOrder(char * filename);

#endif