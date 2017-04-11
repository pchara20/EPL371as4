#include "webServer.h"

/*****************************************************************************/
/**
 ** Function to determine which option is requested from the received message of
 ** the browser. We return the proper number based on the type of the request.
 ** If the request is not known or one that was not implemented we return -1.
 */
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

/*****************************************************************************/
/**
 ** Function where we receive the request message from the browser and this
 ** header is saved in a buffer based on the accepted file descriptor due to the
 ** fact that we work with a thread pool there may be more than 1 request. We
 ** tokenize the response message and in a struct we save the Type of the 
 ** request(GET, HEAD, DELETE), the fileName, the Protocol(HTTP/1.1), the 
 ** connection type, the Host, the content-length, the content-type based on the
 ** extension of the file and the file descriptor where the accepted message was
 ** assigned. After we assign these values to the struct created we create a
 ** node and insert it in our queue of requests to be served. If a thread is
 ** available it serves the request immedietly. If the option -d is given for
 ** debug it shows the process and how the messages are received and inserted in
 ** the queue. 
 */
void insertion(int acceptfd) {

	char tokens[500]; // tokenize each line
	int tokensCounter = 0; // counter for tokens
	char line[500]; // array to hold each line
	char tokenMatched[500]; // array to hold token matched
	struct request *incomingRequest = NULL; // incoming request
	char in_buf[BUF_SIZE]; // buffer array
	int retcode; // return code after accepting
	memset(in_buf, 0, sizeof(in_buf)); // empty the buffer
	// read from socket and save in buffer
	retcode = recv(acceptfd, in_buf, BUF_SIZE, 0);

	if (retcode < 0) { // if error in receiving
		printf("Error in receiving!");
	} else {
		if (strlen(in_buf) == 0) { // if incoming message is empty
			// reset arrays
			memset(tokens, 0, sizeof(tokens));
			memset(line, 0, sizeof(line));
			memset(tokenMatched, 0, sizeof(tokenMatched));
			memset(in_buf, 0, sizeof(in_buf));
			// and return
			return;
		}

		if (debug_flag == 1) { // if debug option is given
			printf("***********************************************\n");
			printf("REQUEST RECEIVED: \n %s\n", in_buf);
			printf("***********************************************\n");
		}
		// allocate memory for incoming request to store information
		incomingRequest = malloc(sizeof(struct request));
		// empty the arrays of the struct
		memset(incomingRequest->connection, 0,
				sizeof(incomingRequest->connection));
		memset(incomingRequest->fileName, 0, sizeof(incomingRequest->fileName));
		memset(incomingRequest->fileType, 0, sizeof(incomingRequest->fileType));
		memset(incomingRequest->protocol, 0, sizeof(incomingRequest->protocol));
		memset(incomingRequest->server, 0, sizeof(incomingRequest->server));
		memset(incomingRequest->type, 0, sizeof(incomingRequest->type));

		int lengthCounter = 0; // counter for length of message
		int lineLengthCounter = 0; // counter for line
		int lineCounter = 0; // the counter to know which information to store
		int token = 0; // token counter to know which token we will store
		// empty arrays
		memset(tokens, 0, sizeof(tokens));
		memset(line, 0, sizeof(line));
		memset(tokenMatched, 0, sizeof(tokenMatched));
		// while there are still information in buffer
		while (lengthCounter < strlen(in_buf)) {
			// while we are still reading the same line
			while (in_buf[lengthCounter] != '\n') {
				// append the character from buffer to line array
				line[lineLengthCounter] = in_buf[lengthCounter];
				// increase counters
				lengthCounter++;
				lineLengthCounter++;
			}
			lineLengthCounter = 0; //reset the counter
			int insideLineCounter = 0; // counter when we are reading the line
			// while there are still information in the current line
			while (insideLineCounter < strlen(line)) {
				// if the character we check is not space
				if (line[insideLineCounter] != ' ') {
					// append the character from line to tokens array
					tokens[tokensCounter] = line[insideLineCounter];
					// increase counters;
					tokensCounter++;
					insideLineCounter++;
				} else { // if we read space
					// we reset the tokensMatched array
					memset(tokenMatched, 0, sizeof(tokenMatched));
					// we copy the token to that array
					strcpy(tokenMatched, tokens);
					if (lineCounter == 0) { // if the line is the first
						if (token == 0) { // if the token is the first
							// we copy the token to the type of the struct
							strcpy(incomingRequest->type, tokens);
							token++; // increase token number
						} else if (token == 1) { // if the token is the second
							int k = 1, j = 0; // counters
							// we remove the first character '/' from file name
							while (k < strlen(tokens)) {
								incomingRequest->fileName[j] = tokens[k];
								k++;
								j++;
							}
							// we end the string
							incomingRequest->fileName[j] = '\0';
							token++;			// and increase the token number
						}
					}
					memset(tokens, 0, sizeof(tokens)); // empty tokens array
					tokensCounter = 0; // reset tokensCounter
					insideLineCounter++; // increase position counter of line
				}
			}
			if (lineCounter == 0) { // if the line is the first
				if (token == 2) { // if the token is the third
					// we copy the token to the protocol field of the struct
					strcpy(incomingRequest->protocol, tokens);
					token++; // increase token number
				}
			}
			// if the field is the connection
			if (strcmp(tokenMatched, "Connection:") == 0) {
				// we save it to the connection field of the struct
				strcpy(incomingRequest->connection, tokens);
				// if the field is the host
			} else if (strcmp(tokenMatched, "Host:") == 0) {
				// we save it to the server field of the struct
				strcpy(incomingRequest->server, tokens);
			}
			memset(tokens, 0, sizeof(tokens)); // empty the tokens array
			tokensCounter = 0; // reset tokensCounter
			lineCounter++; // increase lineCounter
			token = 0; // reset token numbers
			memset(line, 0, sizeof(line)); // empty line array
			lengthCounter++; // increase lengthCounter read from message
			// if we are done and received all fields
			if (strcmp(incomingRequest->type, "") != 0
					&& strcmp(incomingRequest->protocol, "") != 0
					&& strcmp(incomingRequest->connection, "") != 0
					&& strcmp(incomingRequest->server, "") != 0)
				break; // we end the loop
		}
		// if the filename is empty we redirect to index.html
		if (strcmp(incomingRequest->fileName, "") == 0) {
			strcpy(incomingRequest->fileName, "index.html");
		}

		struct stat st;
		// find the size of the file
		if (lstat(incomingRequest->fileName, &st) == 0) {
			incomingRequest->fileLength = st.st_size;
			// work out the file type and check we support it
			int fileLength = strlen(incomingRequest->fileName);
			int i;
			// check the extensions struct and find the extension of the file
			for (i = 0; extensions[i].ext != 0; i++) {
				int length = strlen(extensions[i].ext);
				if (!strncmp(&incomingRequest->fileName[fileLength - length],
						extensions[i].ext, length)) {
					// and copy it to the field of the struct
					strcpy(incomingRequest->fileType, extensions[i].filetype);
					break;
				}
			}
		} else { // if file does not exist
			// file length is set to max
			incomingRequest->fileLength = sizeof(incomingRequest->fileName);
			// and file type is set to text/html
			strcpy(incomingRequest->fileType, "text/html");
			// and print an error that file was not found on server
			if (debug_flag == 1)
				perror("Cannot find file!");
		}
		// set accepted file descriptor
		incomingRequest->acceptfd = acceptfd;
		if (debug_flag == 1) { // if debug
			printf("/*****************************************************/\n");
			printf("ENQUEUED\n");
			printf("Type: %s\n", incomingRequest->type);
			printf("File: %s\n", incomingRequest->fileName);
			printf("Protocol: %s\n", incomingRequest->protocol);
			printf("Connection: %s\n", incomingRequest->connection);
			printf("Server: %s\n", incomingRequest->server);
			printf("Content-Length: %d\n", incomingRequest->fileLength);
			printf("Content-Type: %s\n", incomingRequest->fileType);
			printf("Accept File Descriptor: %d\n", incomingRequest->acceptfd);
			printf("/*****************************************************/\n");
		}
		// create a new node to be enqueued in our queue
		struct node *newNode = (struct node*) malloc(sizeof(struct node));
		// the first field is the information stored from the received message
		newNode->Request = *incomingRequest;
		// and the next node is set to null
		newNode->n_next = NULL;
		// if queue is empty
		if (head == NULL) {
			head = newNode; // we set head to our created node
		} else { 
			struct node *current = head;
			// we pass through our queue to find the end of it
			while (current->n_next != NULL) {
				current = current->n_next;
			}
			// and assign the node at the end
			current->n_next = newNode;
		}
	}
	// we reset the arrays
	memset(tokens, 0, sizeof(tokens));
	memset(line, 0, sizeof(line));
	memset(tokenMatched, 0, sizeof(tokenMatched));
	memset(in_buf, 0, sizeof(in_buf));
}

/*****************************************************************************/
/**
 ** Function where a thread serves a request from the queue. The thread that
 ** arrives first locks the mutex as it needs to get a request from the queue
 ** so only one thread at a time may do this. While our queue is empty the
 ** threads wait there due to the cond variable. When a request arrives the
 ** thread may continue to serve the request. Based on the type of the request
 ** which is included in the struct mentioned above we serve each request
 ** accordingly. When the request is served the thread sleeps, we close the
 ** file descriptor assigned to said thread, we remove the served request from
 ** the queue, we increase the value of the semaphore and we unlock the mutex. 
 */
void *thread_serve() {

	int err; // check for validity of lock and waiting
	while (1) { // run forever
		// if lock fails end the program
		if ((err = pthread_mutex_lock(&mutex))) {
			printf("pthread_mutex_lock: %s\n", strerror(err));
			exit(1);
		}
		//check if queue is empty
		while (head == NULL)
			// wait until queue is created and a node needs to be served
			if ((err = pthread_cond_wait(&cond, &mutex))) {
				printf("phtread_cond_wait: %s\n", strerror(err));
				exit(1);

			}

		struct node *temp = head; // create a temporary node
		head = head->n_next; // assign head to the next node
		if (debug_flag == 1) { // if debug print the node dequeued
			printf("/*****************************************************/\n");
			printf("DEQUEUED\n");
			printf("Type: %s\n", temp->Request.type);
			printf("File: %s\n", temp->Request.fileName);
			printf("Protocol: %s\n", temp->Request.protocol);
			printf("Connection: %s\n", temp->Request.connection);
			printf("Server: %s\n", temp->Request.server);
			printf("Content-Length: %d\n", temp->Request.fileLength);
			printf("Content-Type: %s\n", temp->Request.fileType);
			printf("Accept File Descriptor: %d\n", temp->Request.acceptfd);
			printf("/*****************************************************/\n");
		}
		char buffer[BUF_SIZE]; // buffer
		int file_fd = -1;	// file descriptor to be opened
		// if it fails to open
		int option = checkOption(&temp->Request);
		//IF OPTION NOT IMPLEMENTED
		if (option == -1) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,
					"%s 501 Not Implemented\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: close\r\nContent-Type: %s\r\n\r\n<html><body><h1 style=\"color:red\">Method Not implemented!</h1></body></html>",
					temp->Request.protocol, temp->Request.fileLength, temp->Request.fileType);
			write(temp->Request.acceptfd, buffer, strlen(buffer));
		} else {	//FUNCTION IS VALID
			if (option != 3) {	//IF TYPE IS GET OR HEAD
				if ((file_fd = open(temp->Request.fileName, O_RDONLY)) == -1) {
					if (debug_flag == 1){
						printf("file_fd open error");
					}
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer,
							"%s 404 Not Found\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n<html><body><h1 style=\"color:red\">Document not Found!</h1></body></html>",
							temp->Request.protocol, temp->Request.fileLength,
							temp->Request.connection, temp->Request.fileType);
					write(temp->Request.acceptfd, buffer, strlen(buffer));
					memset(buffer, 0, sizeof(buffer));
				} else {
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer,
							"%s 200 OK\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
							temp->Request.protocol, temp->Request.fileLength,
							temp->Request.connection, temp->Request.fileType);

					write(temp->Request.acceptfd, buffer, strlen(buffer));
					memset(buffer, 0, sizeof(buffer));
					if (option == 1) {
						int ret;
						while ((ret = read(file_fd, buffer, BUF_SIZE)) > 0) {
							write(temp->Request.acceptfd, buffer, ret);
						}
					}
				}
				close(file_fd);
			}
			else {	//IF OPTION IS DELETE
				if ((file_fd = open(temp->Request.fileName, O_RDONLY)) == -1) {
					if (debug_flag == 1){					
						printf("file is not found\n");
						printf("file open error\n");
					}
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer,
							"%s 404 Not Found\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n<html><body><h1 style=\"color:red\">Document not Found!</h1></body></html>",
							temp->Request.protocol, temp->Request.fileLength,
							temp->Request.connection, temp->Request.fileType);
					write(temp->Request.acceptfd, buffer, strlen(buffer));
					memset(buffer, 0, sizeof(buffer));
				} else {
					FILE *f = fopen(temp->Request.fileName, "w");
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer,
							"%s 200 OK\r\nServer: my_Server\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
							temp->Request.protocol, temp->Request.fileLength,
							temp->Request.connection, temp->Request.fileType);
					printf("\n buffer is: %s", buffer);
					write(temp->Request.acceptfd, buffer, strlen(buffer));
					memset(buffer, 0, sizeof(buffer));
					fclose(f);

				}
				close(file_fd);
			}
		}
		// sleep the thread
		sleep(1);
		// close the accept file descriptor
		close(temp->Request.acceptfd);
		// free the node
		free(temp);
		// increase the semaphore counter
		sem_post(&sem);
		// unlock the queue mutex
		if ((err = pthread_mutex_unlock(&mutex))) {
			printf("pthread_mutex_unlock: %s\n", strerror(err));
			exit(1);
		}		
	}
}

/*****************************************************************************/
/**
 ** Function to print helping information when -h is given.
 */
void print_help_options() {
	printf("\nWelcome to our help section of our multi-threaded web server!\n");
	printf("*************************************************************\n");
	printf("Options Supported:\n\t-h\t\tShow help\n\t");
	printf("-n thread_num\tChange number of threads (Default is 40)\n\t");
	printf("-p port_num\tChange number of port (Default is 30000)\n\t");
	printf("-r dir_name\tChange working directory (Default is current)\n\t");
	printf("-d\t\tShow debugging information\n\n");
}

/*****************************************************************************/

int main(int argc, char *args[]) {
	// initialize mutexes
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&smutex, NULL);

	int sockfd; // socket file descriptor
	char *dir; // change to directory if needed
	file = malloc(sizeof(char *)); // allocate memory for filename
	dir = malloc(sizeof(char *)); // allocate memory for directory name

	int i;
	int help_flag = 0, dir_flag = 0; // options supported

	for (i = 0; i < argc; i++) {
		if (strcmp(args[i], "-h") == 0) { // show help
			help_flag = 1;
		} else if (strcmp(args[i], "-n") == 0) {
			threadnum = atoi(args[i + 1]); // change number of threads
										   // default is 40
		} else if (strcmp(args[i], "-p") == 0) {
			portnum = atoi(args[i + 1]); // change number of port
									     // default is 8080
		} else if (strcmp(args[i], "-r") == 0) {
			dir_flag = 1;
			dir = args[i + 1]; // change to directory given
		} else if (strcmp(args[i], "-d") == 0) {
			debug_flag = 1; // show debugging information
		}
	}
	// array of threads
	pthread_t t_serve[threadnum];
	// initialize semaphore with number of threads value
	sem_init(&sem, 0, threadnum);

	if (help_flag == 1) {
		print_help_options(); // print help
		exit(1); // and exit
	} else if (dir_flag == 1) { // change directory
		if (chdir(dir) < 0) {
			perror("Directory does not exist");
			exit(1);
		}
	}

	struct sockaddr_in serv_addr; // struct containing the socket information
	sockfd = socket(AF_INET, SOCK_STREAM, 0); //creation of socket
	if (sockfd < 0) {
		perror("error creating socket");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // Internet domain
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portnum); // set port
	// bind the socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("binding error");
		exit(1);
	}

	/*LISTENER*/
	unsigned int acceptfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr;
	clilen = sizeof(cli_addr);
	int err;
	listen(sockfd, 64);	//socket is ready to use
	int w;
	// create the thread pool
	for (w = 0; w < threadnum; w++) {
		pthread_create(&t_serve[w], NULL, thread_serve, NULL);
	}

	while (1) {	//run forever
		sem_wait(&sem); // only 40 threads can run at the same time
		// accept the connection
		acceptfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (acceptfd < 0) { // if error in accepting
			perror("error in accepting");
			exit(1); // end
		} else {
			// the thread can lock the queue mutex
			pthread_mutex_lock(&mutex);
			// and insert in queue
			insertion(acceptfd);
			// it can then signal the thread to continue to dequeue
			if ((err = pthread_cond_signal(&cond))) {
				printf("pthread_cond_signal: %s\n", strerror(err));
				exit(1);
			}
			// when done unlock the mutex
			pthread_mutex_unlock(&mutex);
		}
	}
}
