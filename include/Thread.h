#pragma once

#include "pthread.h"
#include <functional>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <iostream>

static void* runThread(void* arg);

/*
 * pthread wrapper class
 */
class Thread {
public:

	Thread() :
					m_id(0)
	{}

	virtual ~Thread() {}

	// initialize thread
	int init() {
		int status = pthread_create(&m_thread, NULL, runThread, this);
		memcpy(&m_id, &m_thread, std::min(sizeof(m_id), sizeof(m_thread)));
		return status;
	}

	// function executed inside of the thread
	virtual void *run() = 0;

	// join thread
	int join() {
		int rc = pthread_join(m_thread, NULL);
		return rc;
	}

	// get id of the thread
	uint64_t ID() {
		if (m_id == 0) init();
		return m_id;
	}

private:
	uint64_t m_id;		// id of the thread
	pthread_t m_thread;	// pthread
};

// wrapping function
static void* runThread(void* arg) {
	return ((Thread*) arg)->run();
}

