#include<fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<pthread.h>
#include<malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<time.h>
#include<semaphore.h>
#define BUF_SIZE 1024

sem_t sem;
int free_thread;
char * file = NULL;

pthread_t t_serve;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct request {
	char type[50];
	char fileName[50];
	char protocol[50];
	char connection[50];
	char fileType[50];
	int fileLength;
	char server[50];
	int acceptfd;
};

struct {
	char *ext;
	char *filetype;
} extensions[] = { { "gif", "image/gif" }, { "jpg", "image/jpg" }, { "jpeg",
		"image/jpeg" }, { "png", "image/png" }, { "ico", "image/ico" }, { "zip",
		"image/zip" }, { "gz", "image/gz" }, { "tar", "image/tar" }, { "htm",
		"text/html" }, { "html", "text/html" }, { 0, 0 } };

struct node {
	struct request Request;
	struct node *n_next;
}*head = NULL;

void insertion(int acceptfd);
void *thread_serve();
void print_help_options();

int checkOption(struct request *incoming);
int checkOption(struct request *incoming) {

	if (strcmp(incoming->type, "GET") == 0)
		return 1;
	else if (strcmp(incoming->type, "HEAD") == 0)
		return 2;
	else if (strcmp(incoming->type, "DELETE") == 0)
		return 3;
	else
		return -1;
}

void insertion(int acceptfd) {
	struct request *incomingRequest = NULL;
	char in_buf[BUF_SIZE];
	int retcode;
	memset(in_buf, 0, sizeof(in_buf));
	retcode = recv(acceptfd, in_buf, BUF_SIZE, 0);	//diavazw apo socket

	if (retcode < 0) {
		printf("recv error detected ...\n");
	} else {

		incomingRequest = malloc(sizeof(struct request));

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
		while (c < strlen(in_buf)) {
			while (in_buf[c] != '\n') {
				line[c1] = in_buf[c];
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
		printf("Acceftfd: %d\n", incomingRequest->acceptfd);

		struct node *newNode = (struct node*) malloc(sizeof(struct node));
		newNode->Request = *incomingRequest;
		newNode->n_next = NULL;

		if (head == NULL) {
			head = newNode;
			printf("Added node in the beggining");
		} else {
			struct node *current = head;
			while (current->n_next != NULL) {
				current = current->n_next;
			}
			current->n_next = newNode;
			printf("Added in the end of list");

		}
	}
}

void *thread_serve() {
	int err;
	while (1) {
		printf("\n Entered serving thread\n");

		/*me ti seira ta threads using mutex mpenoun sto func*/
		if ((err = pthread_mutex_lock(&mutex))) {
			printf("pthread_mutex_lock: %s\n", strerror(err));
			exit(1);
		}
		printf("empika kai perimenw na mpei kati sto queue\n");

		//check if queue is empty
		while (head == NULL)
			//wait na mpei kati mesa sto queue
			if ((err = pthread_cond_wait(&cond, &mutex))) {
				printf("phtread_cond_wait: %s\n", strerror(err));
				exit(1);

			}

		struct node *temp = head;
		head = head->n_next;
		char buffer[BUF_SIZE];
		int file_fd;	//DESCRIPTOR TOU FILE POU ENA ANIKSI
		if ((file_fd = open(temp->Request.fileName, O_RDONLY)) == -1)
			printf("file_fd open error");
		else {
//			printf("\nKati mpike sto queue\n");
//			memset(buffer, 0, BUF_SIZE);
//			strcpy(buffer,temp->Request.protocol);
//			strcat(buffer," 200 OK\r\nServer: my_server\r\nContent-Length: ");
//			//strcat(buffer,temp->Request.fileLength);
//			strcat(buffer,"\r\nConnection: ");
//			strcat(buffer,temp->Request.connection);
//			strcat(buffer,"\r\nContent-Type: ");
//			strcat(buffer,temp->Request.fileType);
//			strcat(buffer,"\r\n\r\n");
//			sprintf(buffer,
//					"%s 200 OK\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
//					temp->Request.protocol, temp->Request.fileLength,
//					temp->Request.connection, temp->Request.fileType);
			strcpy(buffer,"HTTP/1.1 404 Not Found\nContent-Type:text/html\n\n<html><body><h1>FILE NOT FOUND</h1></body></html>");
			printf("%s\n", buffer);
			write(temp->Request.acceptfd, buffer, strlen(buffer));
//			memset(buffer, 0, sizeof(buffer));
//
//			int ret;
//			while ((ret = read(file_fd, buffer, BUF_SIZE)) > 0) {
//				write(temp->Request.acceptfd, buffer, ret);
//			}
			sleep(1);

			if ((err = pthread_mutex_unlock(&mutex))) {
				printf("pthread_mutex_unlock: %s\n", strerror(err));
				exit(1);
			}
			sem_post(&sem);
		}
	}
}

int main(int argc, char *args[]) {
	pthread_t t_serve[10];
	int sockfd;
	char *dir;
	file = malloc(sizeof(char *));
	dir = malloc(sizeof(char *));

	int portnum = 8087, threadnum = 10;
	int i;
	int help_flag = 0, dir_flag = 0;

// Parser code
	for (i = 0; i < argc; i++) {
		if (strcmp(args[i], "-h") == 0) {
			help_flag = 1;
		} else if (strcmp(args[i], "-n") == 0) {//n for number of max threads
			threadnum = atoi(args[i + 1]);
		} else if (strcmp(args[i], "-p") == 0) {		//p for port number
			portnum = atoi(args[i + 1]);
		} else if (strcmp(args[i], "-r") == 0) {		//r for directory
			dir_flag = 1;
			dir = args[i + 1];
		}

	}

	sem_init(&sem, 0, threadnum); //init semaphore me initial value threadnum

//Parser code ends

	if (help_flag == 1)	// printing help options and exit if -h option is specified
			{
		print_help_options();
		exit(1);
	} else if (dir_flag == 1)	//changing directory if -d option is specified
			{
		if (chdir(dir) < 0) {
			perror("\nDirectory doesnt exist");
			exit(1);
		}
	}

	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);			//creation of socket
	if (sockfd < 0)
		perror("error creating socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //internet domain
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portnum); //kathorizw port

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //binding socket
		perror("binding error");

	/*LISTENER*/
	unsigned int acceptfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	clilen = sizeof(cli_addr);
	int err;
	int running_threads = 0;
	listen(sockfd, 64);				//socket is read to use

	while (1) {				//run forever
		sem_wait(&sem);
		acceptfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (acceptfd < 0)
			perror("error in accepting");
		else {

			printf("klidonw mutex");
			pthread_mutex_lock(&mutex);
			pthread_create(&t_serve[running_threads], NULL, &thread_serve,
			NULL);
			insertion(acceptfd);
			/*send signal to condition*/
			printf("stelnw sima condition");
			if ((err = pthread_cond_signal(&cond))) {

				printf("pthread_cond_signal: %s\n", strerror(err));
				exit(1);
			}
			printf("kseklidonw mutex\n");
			pthread_mutex_unlock(&mutex);
			printf("inserted in queue\n");
			sleep(1);
		}
		/*LISTENER*/
	}
}

void print_help_options() {
	printf(
			"\n−d : Enter debugging mode. That is, do not daemonize, only accept one connection at a \ntime and enable logging to stdout. Without this option, the web server should run as a daemon process in the background. \n−h : Print a usage summary with all options and exit. \n−l file : Log all requests to the given file. See LOGGING for details.\n−p port : Listen on the given port. If not provided, myhttpd will listen on port 8080. \n−r dir : Set the root directory for the http server to dir. \n−t time : Set the queuing time to time seconds. The default should be 60 seconds. \n−n threadnum: Set number of threads waiting ready in the execution thread pool to threadnum. \nThe default should be 4 execution threads. \n−s sched: Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS.");
}
