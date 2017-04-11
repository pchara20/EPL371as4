########################################################
# Makefile for compiling the program skeleton

# 'make' build executable file 'PROJ'

# 'make clean'	removes all .o executable 

########################################################

PROJ = webServer 		# name of the project

CC = gcc 		# name of the compiler

# define any compile-time flags

CFLAGS = -g -Wall

LFLAGS = -lm -lpthread

# list of object files

HEADERS = $(wildcard *.h)

C_FILES = $(wildcard *.c)

OBJS = $(patsubst %.c, %.o, $(C_FILES))

# to create the executable file we need the individual object files

$(PROJ): $(OBJS)
	
	$(CC) $(OBJS) -Wall $(LFLAGS) -o $@

# to create each individual object file we need to compile these files

# using the following general purpose macro

%.o : %.c $(HEADERS)
	
	$(CC) $(CFLAGS) -c $< -o $@

# to clean .o files: "make clean"

clean:	
	
	rm -rf *.o
	rm -rf webServer
