#pragma once

#include "pthread.h"

class Cond;

/*
 * pthread mutex wrapper
 */
class Mutex {
public:

	// make a condition wrapper as friend class
	friend class Cond;

	// constructor
	Mutex() {
		pthread_mutex_init(&m_mutex, NULL);
		unlock();
	}

	// destructor
	~Mutex() {
		pthread_mutex_unlock(&m_mutex);
		pthread_mutex_destroy(&m_mutex);
	}

	// lock mutex
	int lock() {
		return pthread_mutex_lock(&m_mutex);
	}

	// trylock mutex
	int trylock() {
		return pthread_mutex_trylock(&m_mutex);
	}

	// unlock mutex
	int unlock() {
		return pthread_mutex_unlock(&m_mutex);
	}

protected:

	pthread_mutex_t m_mutex; 	// internal mutex
};

/*
 * pthread condition wrapper
 */
class Cond {
public:

	// make a mutex wrapper as friend class
	friend class Mutex;

	// constructor
	Cond() {
		pthread_cond_init(&m_cond, NULL);
	}

	// destructor
	~Cond() {
		pthread_cond_destroy(&m_cond);
	}

	// send signal to the condition var
	int sendSignal() {
		return pthread_cond_signal(&m_cond);
	}

	// wait for signal
	int wait(Mutex* mutex) {
		return pthread_cond_wait(&m_cond, &(mutex->m_mutex));
	}

protected:

	pthread_cond_t m_cond; // internal condition variable
};
