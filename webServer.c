/*
 * webServer.c
 *
 *  Created on: 7 Apr 2017
 *      Author: acosti
 */

#include "webServer.h"
/*****************************************************************************/
sem_t semaphore; // we use a semaphore for 40 available threads
char *file = NULL;
pthread_t thread[MAX_THREADS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
/*****************************************************************************/
typedef struct {
	char type[50];
	char *url;
	char protocol[10];
	char *server;
	char *host;
	char connection[15];
} request;
/*****************************************************************************/
request *incomingRequest = NULL;
/*****************************************************************************/

int checkOption(request *incoming){

	if (strcmp(incoming->type, "GET") == 0)
		return 1;
	else if (strcmp(incoming->type, "HEAD") == 0)
		return 2;
	else if (strcmp(incoming->type, "DELETE") == 0)
		return 3;
	else
		return -1;
}

/*****************************************************************************/
int main(int argc, char *argv[]){

	incomingRequest = malloc(sizeof(request));

	incomingRequest->url = malloc(sizeof(char* ) * 256);
	incomingRequest->server = malloc(sizeof(char* ) * 256);
	incomingRequest->host = malloc(sizeof(char* ) * 256);

	strcpy(incomingRequest->type, "GET");
	int x = 0;
	while (incomingRequest->type[x] != '\0')
		x++;
	incomingRequest->type[x] = '\0';

	printf("Type: %s\n", incomingRequest->type);
	int function = checkOption(incomingRequest);
	printf("Function: %d\n", function);
	return 0;
}
