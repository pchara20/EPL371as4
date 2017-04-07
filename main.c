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
#define OK_IMAGE    "HTTP/1.1 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.1 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.1 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"

sem_t sem;
int free_thread;
char * file = NULL;

pthread_t t_serve;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void insertion(int acceptfd, char *filename, int file_size, unsigned int ip,
		char * in_buf, char * type);
void *thread_serve();
void print_help_options();
void *thread_serve() {
	while (1) {

		printf("\n Entered serving thread\n");
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);

		printf("\nGot signal\n");

		//struct request r = piase to proto item apo ti lista
		//gemose to R me ta stoixia ekinou tou node
		//kane free to node
		pthread_mutex_unlock(&mutex);
		printf("\n serving thread unlocked mutex");

		//dimiourga to header (costi)
		//analoga tipose content an xriastei GET)
		sem_post(&sem);
	}
}

void *thread_listen(void *arg) {
	unsigned int sockfd = *((unsigned int*) arg);
	unsigned int acceptfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	clilen = sizeof(cli_addr);

	int retcode;
	off_t file_size;
	char in_buf[BUF_SIZE];

	char *fname = malloc(sizeof(char *));
	struct stat st;
	int k, j;

	listen(sockfd, 5);				//socket is read to use
	while (1) {
		acceptfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		printf("AKOUSA KATI\n");
		if (acceptfd < 0)
			perror("error in accepting");

		unsigned int ip = cli_addr.sin_addr.s_addr; //piannw IP address client

		memset(in_buf, 0, sizeof(in_buf));
		retcode = recv(acceptfd, in_buf, BUF_SIZE, 0);	//diavazw apo socket
		char *file_name = malloc(sizeof(char *));
		char *type = malloc(sizeof(char *));
		printf("Akousa to:\n%s\n", in_buf);
		if (retcode < 0) {
			printf("recv error detected ...\n");
		} else {
			type = strtok(in_buf, " ");
			file_name = strtok(NULL, " ");
		}

		if ((file_name != NULL) && (type != NULL)) {

			printf("Filename:%s\n", file_name);
			printf("Type:%s\n", type);

			k = 1, j = 0;
			while (k < strlen(file_name)) {
				fname[j] = file_name[k];
				k++;
				j++;
			}

			if (stat(fname, &st) == 0)
				file_size = st.st_size;

			pthread_mutex_lock(&mutex);
			insertion(acceptfd, file_name, file_size, ip, in_buf, type);
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
			printf("inserted in queue");

		} else {
			continue;
		}
	}
}

void insertion(int acceptfd, char *filename, int file_size, unsigned int ip,
		char * in_buf, char * type) {
	///

	//new=(//filakse xoro gia ena node);
	//gemose to node

//	if (head==NULL)
//		head=to_node;
//	else{
//		rear->link=new;
//		rear=new;
//	}

}

int main(int argc, char *args[]) {
	pthread_t t_listener, t_serve[10];
	int sockfd, ids;
	char *dir;
	file = malloc(sizeof(char *));
	dir = malloc(sizeof(char *));

	int portnum = 8080, threadnum = 40;
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

	int w;
	for (w = 0; w < threadnum; w++) {
		pthread_create(&t_serve[w], NULL, &thread_serve, NULL);
	}

	ids = sockfd;
	printf("\nWill now create listener");
	pthread_create(&t_listener, NULL, &thread_listen, &ids); //creating listener thread
	pthread_join(t_listener, NULL);
	printf("\nAfter join in main");

	close(sockfd);
	return 0;
}

void print_help_options() {
	printf(
			"\n−d : Enter debugging mode. That is, do not daemonize, only accept one connection at a \ntime and enable logging to stdout. Without this option, the web server should run as a daemon process in the background. \n−h : Print a usage summary with all options and exit. \n−l file : Log all requests to the given file. See LOGGING for details.\n−p port : Listen on the given port. If not provided, myhttpd will listen on port 8080. \n−r dir : Set the root directory for the http server to dir. \n−t time : Set the queuing time to time seconds. The default should be 60 seconds. \n−n threadnum: Set number of threads waiting ready in the execution thread pool to threadnum. \nThe default should be 4 execution threads. \n−s sched: Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS.");
}
