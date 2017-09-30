CC=g++

CFLAGS=-std=c++11 -Wall
LDFLAGS=-static -lmp3lame -pthread #-lmp3lame

SRCDIR = src

INCDIR = include
INCLAME = lame
SOURCES := $(wildcard $(SRCDIR)/*.cpp) #WavConverter.cpp DataCollector.cpp DataDistributor.cpp FileHandler.cpp MutexManager.cpp QueueManager.cpp stdafx.cpp ThreadManager.cpp TreadManager.cpp TreadWorker.cpp WavDecoder.cpp

all:
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(INCLAME) $(SOURCES) -o WavConverter $(LDFLAGS)   
