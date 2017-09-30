#pragma once

#include "DataManager.h"

#include "ThreadFactory.h"
#include "ThreadWorker.h"

#include <pthread.h>
#include <thread>
#include <vector>
#include <list>

#ifdef WIN32
#include "Windows.h"
#endif

/*
 * Thread manager interface
 */
class ThreadManager {
public:
	virtual ~ThreadManager() {
	}
	virtual void init(ThreadFactory* factory) =0;
	virtual void runThreads() = 0;
	virtual void joinThreads() = 0;
	virtual void addFile(std::string file) = 0;
	virtual void terminate() = 0;
	virtual void setNoItemsToProcess(bool state) = 0;
	// get number of available cores
	static int getNCores() {
#ifdef WIN32
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);
		return siSysInfo.dwNumberOfProcessors;
#else
		return std::thread::hardware_concurrency();
#endif
	}
};

/*
 * Implementation of ThreadManager interface
 */
template<typename DataManager>
class ThreadManagerImpl: public ThreadManager {
public:

	// constructor
	ThreadManagerImpl() {
		m_nThreads = ThreadManagerImpl::getNCores();
		printf("number of threads: %i\n", m_nThreads);
	}

	// destructor
	virtual ~ThreadManagerImpl() {
	}

	// create ThreadWorkers
	void init(ThreadFactory* factory) {
		for (int ii = 0; ii < m_nThreads; ii++) {
			m_threadWorkers.push_back(
					(ThreadWorker*) factory->createThread(&m_dataManager));
		}
	}

	// start ThreadWorkers
	void runThreads() {
		for (ThreadWorker* item : m_threadWorkers) {
			item->init();
			m_dataManager.initThread(item->ID());
		}
	}

	// join threads
	void joinThreads() {
		for (ThreadWorker* item : m_threadWorkers)
			item->join();
	}

	// add file to the processing list
	void addFile(std::string file) {
		m_dataManager.addFile(file);
	}

	// delete ThreadWorkers
	void terminate() {
		for (ThreadWorker* item : m_threadWorkers) {
			delete item;
		}
		m_threadWorkers.clear();
	}

	// set true when list of items is over
	void setNoItemsToProcess(bool state) {
		m_dataManager.setNoItemsToProcess(state);
	}

private:

	DataManager m_dataManager;					// DataManager object
	int m_nThreads;								// number of threads
	std::list<ThreadWorker*> m_threadWorkers;	// ThreadWorkers list

};

// Parallel processing
class ThreadManagerParallel: public ThreadManagerImpl<DataManagerParallel> {
public:
	ThreadManagerParallel() {
		printf("Set processing to the parallel mode!\n");
	}
};

// Concurrent processing
class ThreadManagerConcurrent: public ThreadManagerImpl<DataManagerConcurrent>
{
public:
	ThreadManagerConcurrent() {
		printf("Set processing to the concurrent mode!\n");
	}
};

