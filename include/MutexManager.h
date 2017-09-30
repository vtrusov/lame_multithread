#pragma once

#include "Mutex.h"
#include "QueueManager.h"

#include <pthread.h>
#include <map>
#include <iostream>
#include <cstdint>

typedef std::map<uint64_t, Mutex*> MutexMap;

/*
 * the class which manages write queue
 */
class MutexManager {
public:

	MutexManager(QueueManager* queueManager);
	~MutexManager();

	// add thread
	void initThread(uint64_t id);

	// lock write for the given thread
	int lockWrite(uint64_t id);

	// trylock write for the given thread
	int trylockWrite(uint64_t id);

	// unlock write for the given thread
	int unlockWrite(uint64_t id);

	// get mutex which block file reading
	Mutex* getReadMutex();

	// get mutex which block file writing
	Mutex* getWriteMutex();

	// unlock first thread in the write queue
	void unlockFirstWrite();

	// lock write access for all threads and unlock first in the queue
	void updateWriteQueue();

	// append thread to write queue
	void insertThreadInWriteQueue(uint64_t id);

	// erase first element in the queue
	void moveQueue();

	// clear queue
	void empty();

private:

	// lock access to the object
	void lockInternalAccess();

	// unlock access to the object
	void unlockInternalAccess();

	Mutex m_internalAccess;		// mutex for internal access
	MutexMap m_writeMutexes;	// mutex map for write access

	Mutex m_readMutex;			// mutex to access to the input file
	Mutex m_writeMutex;			// mutex to access to the output file

	QueueManager* m_queueManager;	// object to hold access queue
};
