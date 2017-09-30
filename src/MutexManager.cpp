#include "MutexManager.h"

MutexManager::MutexManager(QueueManager* queueManager) :
				m_queueManager(queueManager)
{}

MutexManager::~MutexManager() {}

void MutexManager::initThread(uint64_t id) {
	m_writeMutexes.insert(MutexMap::value_type(id, new Mutex()));
}

int MutexManager::lockWrite(uint64_t id) {
	return m_writeMutexes.at(id)->lock();
}

int MutexManager::trylockWrite(uint64_t id) {
	return m_writeMutexes.at(id)->trylock();
}

int MutexManager::unlockWrite(uint64_t id) {
	return m_writeMutexes.at(id)->unlock();
}

Mutex* MutexManager::getReadMutex() {
	return &m_readMutex;
}

Mutex* MutexManager::getWriteMutex() {
	return &m_writeMutex;
}

void MutexManager::unlockFirstWrite() {
	unlockWrite(m_queueManager->getFirstInTheList());
}

void MutexManager::updateWriteQueue() {
	lockInternalAccess();
	for (uint64_t id : m_queueManager->getQueue()) {
		lockWrite(id);
	}

	unlockWrite(m_queueManager->getFirstInTheList());
	unlockInternalAccess();
}

void MutexManager::insertThreadInWriteQueue(uint64_t id) {
	lockInternalAccess();
	m_queueManager->addElementToQueue(id);
	trylockWrite(id);
	unlockFirstWrite();
	unlockInternalAccess();
}

void MutexManager::moveQueue() {
	lockInternalAccess();
	m_queueManager->moveQueue();
	unlockWrite(m_queueManager->getFirstInTheList());
	unlockInternalAccess();
}

void MutexManager::empty() {
	lockInternalAccess();
	for (auto &element : m_writeMutexes) {
		unlockWrite(element.first);
	}
	m_queueManager->clearQueue();
	unlockInternalAccess();
}

void MutexManager::lockInternalAccess() {
	m_internalAccess.lock();
}

void MutexManager::unlockInternalAccess() {
	m_internalAccess.unlock();
}
