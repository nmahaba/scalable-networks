#!/bin/bash

CC = g++
CFLAGS = -g -lpthread
INCLUDES = -I. -I../Common/. -I../SocketUtilities/. -I../FileUtilities/. -I../ThreadUtilities/.
SOURCES = Main.cpp ../SocketUtilities/SocketUtilities.cpp ../FileUtilities/FileUtilities.cpp ../ThreadUtilities/ThreadUtilities.cpp
OUTPUT = main.out

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT)

clean:
	rm -f *.o *.out
