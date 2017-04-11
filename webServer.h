#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#define BUF_SIZE 1024

sem_t sem; // semaphore
char *file = NULL; // file to check

int debug_flag = 0; // debug flag
int threadnum = 40; // thread number
int portnum = 30000; // set port number
static pthread_mutex_t mutex; // queue mutex
static pthread_mutex_t smutex; // thread mutex
static pthread_cond_t cond; // condition variable if queue is empty

/*****************************************************************************/
// struct containing the fields of a message
struct request {
	char type[500];
	char fileName[500];
	char protocol[500];
	char connection[500];
	char fileType[500];
	int fileLength;
	char server[500];
	int acceptfd;
};

/*****************************************************************************/
// struct containing the extensions supported
struct {
	char *ext;
	char *filetype;
} extensions[] = { 
	{ "gif", "image/gif" }, 
	{ "jpg", "image/jpg" }, 
	{ "jpeg", "image/jpeg" }, 
	{ "pdf", "application/pdf" }, 
	{ "txt", "text/plain" }, 
	{ "sed", "text/plain" }, 
	{ "awk", "text/plain" }, 
	{ "c", "text/plain" },
	{ "h", "text/plain" }, 
	{"htm","text/html" }, 
	{ "html", "text/html" }, 
	{ 0, "application/octet-stream" } 
};

/*****************************************************************************/
// the node to be inserted in queue
struct node {
	struct request Request;
	struct node *n_next;
}*head = NULL; // initialize head as NULL

/*****************************************************************************/
void insertion(int acceptfd);
void *thread_serve();
void print_help_options();
int checkOption(struct request *incoming);
/*****************************************************************************/

#endif /* WEBSERVER_H_ */
