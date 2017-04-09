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

struct {
	char *ext;
	char *filetype;
} extensions[] = { { "gif", "image/gif" }, { "jpg", "image/jpg" }, { "jpeg",
		"image/jpeg" }, { "png", "image/png" }, { "ico", "image/ico" }, { "zip",
		"image/zip" }, { "gz", "image/gz" }, { "tar", "image/tar" }, { "htm",
		"text/html" }, { "html", "text/html" }, { 0, 0 } };
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
			"GET /index.zip HTTP/1.1\nHost: 127.0.0.1:30047\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1";

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
						int k = 1, j = 0;
						while (k < strlen(tokens)) {
							incomingRequest->fileName[j] = tokens[k];
							k++;
							j++;
						}
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

	if (strcmp(incomingRequest->fileName, "") == 0){
		strcpy(incomingRequest->fileName, "index.html");
	}

	struct stat st;
	if (lstat(incomingRequest->fileName, &st) == 0)
		incomingRequest->fileLength = st.st_size;
	else {
		perror("Cannot find file!\n");
		exit(1);
	}

	/* work out the file type and check we support it */
	int fileLength = strlen(incomingRequest->fileName);
	int i;
	for (i = 0; extensions[i].ext != 0; i++) {
		int length = strlen(extensions[i].ext);
		if (!strncmp(&incomingRequest->fileName[fileLength - length],
				extensions[i].ext, length)) {
			strcpy(incomingRequest->fileType, extensions[i].filetype);
			break;
		}
	}

	printf("Type: %s\n", incomingRequest->type);
	printf("File: %s\n", incomingRequest->fileName);
	printf("Protocol: %s\n", incomingRequest->protocol);
	printf("Connection: %s\n", incomingRequest->connection);
	printf("Server: %s\n", incomingRequest->server);
	printf("Content-Length: %d\n", incomingRequest->fileLength);
	printf("Content-Type: %s\n", incomingRequest->fileType);
	return 0;
}
