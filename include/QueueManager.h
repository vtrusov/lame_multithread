#pragma once

#include "pthread.h"
#include <vector>
#include <cstdint>

/*
 * class which holds thread access queue
 */
class QueueManager {
public:

	typedef std::vector<uint64_t> Queue;

	QueueManager() {}
	~QueueManager() {}

	// get the first thread in the access queue
	uint64_t getFirstInTheList() const {
		return m_accessQueue.front();
	}

	// append thread to the queue
	void addElementToQueue(uint64_t element) {
		m_accessQueue.push_back(element);
	}

	// erase first element on the queue
	void moveQueue() {
		m_accessQueue.erase(m_accessQueue.begin());
	}

	// get queue
	Queue& getQueue() {
		return m_accessQueue;
	}

	// clear queue
	void clearQueue() {
		m_accessQueue.clear();
	}

private:
	Queue m_accessQueue;	// vector to hold thread queue
};

