/*
 * webServer.h
 *
 *  Created on: 7 Apr 2017
 *      Author: acosti
 */

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
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>

#define BUF_SIZE 1024
#define MAX_THREADS 40

#define IMAGE_OK    "HTTP/1.1 200 OK\nContent-Type:image/gif\n\n"
#define HTML_OK     "HTTP/1.1 200 OK\nContent-Type:text/html\n\n"
#define NOT_FOUND   "HTTP/1.1 404 Not Found\nContent-Type:text/html\n\n"
#define NOT_IMPL	"HTTP/1.1 501 Not Implemented"

int checkOption();
#endif /* WEBSERVER_H_ */
