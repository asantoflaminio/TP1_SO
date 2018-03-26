#ifndef ORDER_H
#define ORDER_H

#include <stdint.h>
#include "types.h"

#define HASHMD5 33

typedef struct {
  	char * filename;
  	boolean processed;
} order_o;

#endif