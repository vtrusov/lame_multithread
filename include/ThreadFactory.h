#pragma once

#include "ConverterConcurrent.h"
#include "ThreadWorker.h"
#include "FileHandler.h"
#include "MP3Manager.h"
#include "MutexManager.h"

#include "Thread.h"
/*
 * interface to the factory
 */
class ThreadFactory {
public:
	ThreadFactory() {}
	virtual ~ThreadFactory() {}
	virtual Thread* createThread(DataManager*) = 0;
};

/*
 * class to create Thread objects
 */
class ThreadWorkerFactory: public ThreadFactory {
public:

	// Base class
	typedef ThreadFactory super;

	ThreadWorkerFactory() {}
	~ThreadWorkerFactory() {}

	// create a new ThreadWorker object
	Thread* createThread(DataManager* dataManager) {
		ThreadWorker* threadWorker = new ThreadWorker(dataManager);
		return threadWorker;
	}
};
