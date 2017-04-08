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
pthread_t t_listener;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

/*****************************************************************************/
typedef struct {
	char type[50];
	char fileName[50];
	char protocol[50];
	char connection[50];
	char fileType[50];
	int fileLength;
	char server[50];
} request;
/*****************************************************************************/
request *incomingRequest = NULL;
/*****************************************************************************/

int checkOption(request *incoming) {

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

int main(int argc, char *argv[]) {

	incomingRequest = malloc(sizeof(request));

	char *x =
			"GET /index.html HTTP/1.1\nHost: 127.0.0.1:30047\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1";

	int c = 0;
	int c1 = 0;
	char tokens[100];
	int tc = 0;
	char line[100];
	int lineCounter = 0;
	int token = 0;
	memset(tokens, 0, sizeof(tokens));
	memset(line, 0, sizeof(line));
	int done = 0;
	while (c < strlen(x)) {
		while (x[c] != '\n') {
			line[c1] = x[c];
			c++;
			c1++;
		}
		printf("line: %s\n", line);
		c1 = 0;
		int p = 0;

		while (p < strlen(line)) {
			if (line[p] != ' ') {
				tokens[tc] = line[p];
				tc++;
				p++;
			} else {
				printf("token 0: %s\n", tokens);
				if (lineCounter == 0) {
					if (token == 0) {
						strcpy(incomingRequest->type, tokens);
						token++;
					} else if (token == 1) {
						strcpy(incomingRequest->fileName, tokens);
						token++;
					}
				}
				memset(tokens, 0, sizeof(tokens));
				tc = 0;
				p++;
			}
		}
		printf("token 1: %s\n", tokens);
		if (lineCounter == 0) {
			if (token == 2) {
				strcpy(incomingRequest->protocol, tokens);
				token++;
			}
		} else if (lineCounter == 1) {
			strcpy(incomingRequest->server, tokens);
		} else if (lineCounter == 6) {
			strcpy(incomingRequest->connection, tokens);
			done = 1;
		}
		memset(tokens, 0, sizeof(tokens));
		tc = 0;

		lineCounter++;
		token = 0;
		printf("\n");
		memset(line, 0, sizeof(line));
		c++;
		if (done == 1)
			break;
	}
	struct stat st;
	if (lstat(incomingRequest->fileName, &st) == 0)
		incomingRequest->fileLength = st.st_size;

	printf("Type: %s\n", incomingRequest->type);
	printf("File: %s\n", incomingRequest->fileName);
	printf("Protocol: %s\n", incomingRequest->protocol);
	printf("Connection: %s\n", incomingRequest->connection);
	printf("Server: %s\n", incomingRequest->server);
	printf("Content-Length: %d\n", incomingRequest->fileLength);
	return 0;
}
