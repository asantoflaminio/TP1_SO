#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "include/application.h"
#include "include/queue.h"
#include "include/order.h"
#include "include/types.h"

#define SEPARATOR "/"

int 
main(int argc, char* argv[]){
	
	if (argc != 3 || (strcmp(argv[1], "hash") != 0)) {
		printf("The arguments were wrong. Format: hash <directory>\n");
		return -1;
	}

	start(argv[2]);
	return 0;
}

void start(const char *dirname){
	int files = 0;
	queue_o orderQueue;

	//Create queue
	printf("Creating order queue...\n");
	orderQueue = newQueue();

	//Fetch files
	printf("Fetching files...\n");
	files = loadFiles(dirname, orderQueue, files);
	printf("Fetchet %d files!\n", files);

	/* Probando que se guarde todo ok en la queue - Despues borrar esto */
	node_o * current = orderQueue->first;
	
	while(current != NULL){
		printf("processed: %d - ", current->order.processed);
		printf("filename: %s \n", current->order.filename);
		current = current->next;
	}
	
	printf("%d", orderQueue->size);
}

int loadFiles(const char *dirname, queue_o queue, int files){
	DIR *dir;
	struct dirent *dp;

	dir = opendir(dirname);
	
	if((dir = opendir(dirname)) != NULL){
		while((dp = readdir(dir)) != NULL){
			if((strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name,"..")!=0)){

				char * current = malloc(strlen(dirname) + strlen(dp->d_name) + strlen(SEPARATOR) + 1);
				strcat(strcat(strcpy(current, dirname), SEPARATOR), dp->d_name);
				
				if(dp->d_type == DT_DIR){
					printf("Opening directory... %s\n", dp->d_name);
					files = loadFiles(current, queue, files);
				} else {
					printf("Opening file %s\n", dp->d_name);
					order_o order;
					order.filename = current;
					order.processed = false;
					enQueue(queue, order);
					files++;
				}
			}
		}
	}

	closedir(dir);
	return files;
}