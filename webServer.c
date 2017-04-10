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
			"GET /index.html HTTP/1.1\nHost: 127.0.0.1:30047\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1";

	char *y =
			"GET /index.html HTTP/1.1\nHost: 127.0.0.1:30002\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\nAccept-Encoding: gzip, deflate, sdch, br\nAccept-Language: en-GB,en;q=0.8,en-US;q=0.6,el;q=0.4";
	int c = 0;
	int c1 = 0;
	char tokens[100];
	int tc = 0;
	char line[100];
	char tokenMatched[100];
	int lineCounter = 0;
	int token = 0;
	memset(tokens, 0, sizeof(tokens));
	memset(line, 0, sizeof(line));
	memset(tokenMatched, 0, sizeof(tokenMatched));

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
				//printf("token 0: %s\n", tokens);
				memset(tokenMatched, 0, sizeof(tokenMatched));
				strcpy(tokenMatched, tokens);
				//printf("token Matched: %s\n", tokenMatched);
				if (lineCounter == 0) {
					if (token == 0) {
						strcpy(incomingRequest->type, tokens);
						printf("Type: %s\n", incomingRequest->type);
						token++;
					} else if (token == 1) {
						int k = 1, j = 0;
						while (k < strlen(tokens)) {
							incomingRequest->fileName[j] = tokens[k];
							k++;
							j++;
						}
						token++;
						printf("File: %s\n", incomingRequest->fileName);
					}
				}
				memset(tokens, 0, sizeof(tokens));
				tc = 0;
				p++;
			}
		}
		//printf("token Matched: %s\n", tokenMatched);

		if (lineCounter == 0) {
			if (token == 2) {
				strcpy(incomingRequest->protocol, tokens);
				printf("Protocol: %s\n", incomingRequest->protocol);
				token++;
			}
		}

		if (strcmp(tokenMatched, "Connection:") == 0) {
			strcpy(incomingRequest->connection, tokens);
			printf("Connection: %s\n", incomingRequest->connection);
		} else if (strcmp(tokenMatched, "Host:") == 0) {
			strcpy(incomingRequest->server, tokens);
			printf("Server: %s\n", incomingRequest->server);
		}

		memset(tokens, 0, sizeof(tokens));
		tc = 0;

		lineCounter++;
		token = 0;
		printf("\n");
		memset(line, 0, sizeof(line));
		c++;
		if (strcmp(incomingRequest->type, "") != 0
				&& strcmp(incomingRequest->protocol, "") != 0
				&& strcmp(incomingRequest->connection, "") != 0
				&& strcmp(incomingRequest->server, "") != 0)
			break;

	}

	if (strcmp(incomingRequest->fileName, "") == 0) {
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

	/*****************************************************************************/
	char buffer[BUF_SIZE];
	int file_fd;	//DESCRIPTOR TOU FILE POU ENA ANIKSI
	if ((file_fd = open(incomingRequest->fileName, O_RDONLY)) == -1)
		printf("file_fd open error");
	else {
		printf("\nKati mpike sto queue\n");
		//stile header
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer,
				"%s 200 OK\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
				incomingRequest->protocol, incomingRequest->fileLength,
				incomingRequest->connection, incomingRequest->fileType);

		printf("buffer: %s\n", buffer);
		//write(acceptfd, buffer, strlen(buffer));
		//printf("tou stelnw to \n%s\n", buffer);
		//write(acceptfd, buffer, strlen(buffer));
		return 0;
	}
}
