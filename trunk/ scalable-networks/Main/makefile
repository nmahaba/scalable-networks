#!/bin/bash

CC = g++
CFLAGS = -g -lpthread
INCLUDES = -I. -I../Common/. -I../SocketUtilities/. -I../FileUtilities/.
#SOURCES= ThreadUtilities.c
SOURCES = Main.cpp ../SocketUtilities/SocketUtilities.cpp ../FileUtilities/FileUtilities.cpp 
OUTPUT = nodes.out

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT)

clean:
	rm -f *.o *.out
