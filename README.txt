----------------------------------Contributors ---------------------------------
							
Andreas Costi (acosti01 - 1003060) 				   							   
Panayiotis Charalampous (pchara20 - 952304)		   							   
Stephanie Nicolaou (snikol07 - 932787)
			   							   
--------------------------------------------------------------------------------
							CONTENTS OF THIS FILE
						Implementation of our Web Server
--------------------------------------------------------------------------------
	
							I. 		Introduction
							II.		Description
							III.	Execution

I.		Introduction
---------------------

In this assignment we had to implement a multi-threaded web server for the HTTP
protocol. For the purpose of this assignment we had to be able to manipulate 
the methods GET, HEAD, DELETE. If a request contained any other method we had to
respond with a "501 Not Implemented" response message. For the GET method our
web server had to retrieve the file requested and preview the content on the 
screen. For the HEAD method our web server had to retrieve the file requested 
but not show the content of the file. For the DELETE method our web server had
to retrieve the file requested and delete the content of the file. In any case
if a file does not exist a 404 Not Found response message is sent. For the GET
method it is shown on the screen.
In the response messages of our web server we stored the information retrieved 
from the header message, such as the Type, Server, Connection, Content-Length, Content-Type, Filename and accepted file descriptor. 
In order to serve many requests at the same time we create a thread-pool where
the default number of threads is 40.

II.		Description
---------------------

In order to implement our web server we firstly initialize our mutexes and our
semaphore. Then we create a socket to open a connection and then we bind this
socket. Then we are ready and listening for requests at this socket. Based on
the number of threads we create a thread-pool ready to server incoming requests.
Then in an endless loop, we decrease the value of the semaphore, and a thread
accepts a request. The thread locks the mutex, receives the request and creates
a struct that holds the information of this request. Then we create a node and 
insert this request in a queue to be served. Due to the fact that this process
is critical it is included in a mutex lock-unlock block. When the insertion 
process completes a cond variable signals that a thread is able to dequeue a
request and process it. The process of dequeuing is critical so it is included
in a mutex lock-unlock block. We then dequeue the node, check the option based
on the type of the request and show the proper message. We then close the 
connection, remove the node from the queue, increase the value of the semaphore
so it is known that a thread is available to process other requests and we 
unlock the mutex.

III.	Execution
---------------------

	Files included: makefile
					webServer.c
					webServer.h
					README.txt
					Report.pdf

*Our program may be run by executing in a terminal the command make.
*Our program may be run by executing in a terminal: 
		gcc -o webServer webServer.c -lpthread ...and then, 
		./webServer [Options Supported:]
					-h				Show help
					-n thread_num	Change number of threads(Default is 40)
					-p port_num		Change number of port(Default is 30000)
					-r dir_name		Change working directory(Default is current)
					-d				Show debugging information
Then in a web browser a request can be given such as 127.0.0.1:30000/a.html
The proper message will be shown.





