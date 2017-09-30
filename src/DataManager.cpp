#include "DataManager.h"

#include "ThreadWorker.h"

ConverterInterface* DataManager::getConverter(ThreadWorker* threadWorker) {
	return threadWorker->m_converter;
}

uint64_t DataManager::getID(ThreadWorker* threadWorker) {
	return threadWorker->ID();
}

DataManagerParallel::DataManagerParallel() :
				m_noItemsToProcess(false)
{
}

DataManagerParallel::~DataManagerParallel() {}

void DataManagerParallel::addFile(std::string file) {
	m_mutex.lock();
	FileHandler* fileHandler = new FileHandler(file);
	fileHandler->resetFilePosition();
	m_input.push_back(fileHandler);
	std::string output = file;
	output.replace(output.length() - 3, 3, "mp3");
	m_output.push_back(new MP3Manager(output));
	m_cond.sendSignal();
	m_mutex.unlock();
}

int DataManagerParallel::getFile(FileHandler* &fileHandler,
		MP3Manager* &mp3Manager)
{
	m_mutex.lock();
	while (m_input.size() == 0) {
		if (m_noItemsToProcess)
			if(m_input.size() == 0) {
			m_mutex.unlock();
			return 0;
		}
		m_cond.wait(&m_mutex);
	}
	fileHandler = m_input.front();
	mp3Manager = m_output.front();
	nextFile();
	m_mutex.unlock();

	return 1;
}

void DataManagerParallel::nextFile() {
	m_input.pop_front();
	m_output.pop_front();
}

void DataManagerParallel::initThread(uint64_t id) {
//	m_mutexManager.initThread(id);
}

int DataManagerParallel::readData(ThreadWorker* threadWorker) {
	int nBlocks(0);
	nBlocks = getConverter(threadWorker)->readBlock();
	return nBlocks;
}

int DataManagerParallel::writeData(ThreadWorker* threadWorker,
		MP3Manager* mp3Manager, int nBlocksWrite)
{
	if (mp3Manager == nullptr) {
		std::cerr << "Output file is not defined!" << std::endl;
		assert(false);
	}

	if (!(mp3Manager->isFileFinalized()))
		mp3Manager->write(getConverter(threadWorker)->getMP3Buffer(),
				nBlocksWrite);
	if (getConverter(threadWorker)->getFinalized()) {
		mp3Manager->setFinalized();
	}

	return 0;
}

ConverterInterface* DataManagerParallel::generateConverter(int pcmMode) {
	ConverterInterface* converter;
	switch (pcmMode) {
	case 1: {
		converter = new ConverterPCM();
	}
		break;
	case 3: {
		converter = new ConverterIEEE();
	}
		break;
	default: {
		assert(false);
	}
	};
	return converter;
}

void DataManagerParallel::deletePointers(FileHandler* &fileHandler,
		MP3Manager* &mp3Manager)
{
	if(fileHandler){
		fileHandler->close();
		delete fileHandler;
		fileHandler = nullptr;
	}
	if(mp3Manager){
		mp3Manager->close();
		delete mp3Manager;
		mp3Manager = nullptr;
	}
}

void DataManagerParallel::setNoItemsToProcess(bool state) {
	m_mutex.lock();
	m_noItemsToProcess = state;
	m_cond.sendSignal();
	m_mutex.unlock();
}

DataManagerConcurrent::DataManagerConcurrent(bool prebuffer) :
				m_prebufferPCM(prebuffer),
				m_queueManager(),
				m_mutexManager(&m_queueManager),
				m_noItemsToProcess(false)
{
}

DataManagerConcurrent::~DataManagerConcurrent() {}

void DataManagerConcurrent::addFile(std::string file) {
	m_mutex.lock();
	FileHandler* fileHandler = new FileHandler(file, m_prebufferPCM);
	fileHandler->resetFilePosition();
	m_input.push_back(fileHandler);
	std::string output = file;
	output.replace(output.length() - 3, 3, "mp3");
	m_output.push_back(new MP3Manager(output));
	m_cond.sendSignal();
	m_mutex.unlock();
}

int DataManagerConcurrent::getFile(FileHandler* &fileHandler,
		MP3Manager* &mp3Manager)
{
	m_mutex.lock();
	if (m_input.size() != 0) nextFile();
	while (m_input.size() == 0) {
		if (m_noItemsToProcess)
			if(m_input.size() == 0) {
			m_mutex.unlock();
			return 0;
		}
		m_cond.wait(&m_mutex);
	}
	fileHandler = m_input.front();
	mp3Manager = m_output.front();
	m_mutex.unlock();
	return 1;
}

void DataManagerConcurrent::nextFile() {
	m_mutexNextFile.lock();

	if (m_output.front() == nullptr) {
		std::cout << "next file (nullptr)" << std::endl;
		m_input.pop_front();
		m_output.pop_front();
		m_mutexManager.empty();
	} else if (m_output.front()->isFileFinalized()) {
		m_input.pop_front();
		m_output.pop_front();
		m_mutexManager.empty();
	}
	m_mutexNextFile.unlock();
}

void DataManagerConcurrent::initThread(uint64_t id) {
	m_mutexManager.initThread(id);
}

int DataManagerConcurrent::readData(ThreadWorker* threadWorker) {
	int nBlocks(0);
	m_mutexManager.getReadMutex()->lock();
	nBlocks = getConverter(threadWorker)->readBlock();
	m_mutexManager.insertThreadInWriteQueue(getID(threadWorker));
	m_mutexManager.getReadMutex()->unlock();
	return nBlocks;
}

int DataManagerConcurrent::writeData(ThreadWorker* threadWorker,
		MP3Manager* mp3Manager, int nBlocksWrite)
{
	if (mp3Manager == nullptr) {
		std::cerr << "Output file is not defined!" << std::endl;
		assert(false);
	}

	m_mutexManager.lockWrite(getID(threadWorker));
	m_mutexManager.getWriteMutex()->lock();

	if (!(mp3Manager->isFileFinalized()))
		mp3Manager->write(getConverter(threadWorker)->getMP3Buffer(),
				nBlocksWrite);
	if (getConverter(threadWorker)->getFinalized())
		mp3Manager->setFinalized(true);

	m_mutexManager.getWriteMutex()->unlock();
	m_mutexNextFile.lock();
	m_mutexManager.moveQueue();
	m_mutexNextFile.unlock();
	return 0;
}

ConverterInterface* DataManagerConcurrent::generateConverter(int pcmMode) {
	ConverterInterface* converter;
	switch (pcmMode) {
	case 1: {
		converter = new ConverterConcurrentPCM(m_prebufferPCM);
	}
		break;
	case 3: {
		converter = new ConverterConcurrentIEEE(m_prebufferPCM);
	}
		break;
	default: {
		assert(false);
	}
	};
	return converter;
}

void DataManagerConcurrent::deletePointers(FileHandler* &fileHandler,
		MP3Manager* &mp3Manager)
{
//	m_mutexNextFile.lock();
//	std::cout << "erased queue size " << m_queueManager.getQueue().size() << std::endl;
//	while (m_queueManager.getQueue().size() != 0) {
////		std::cout << "wait for queue to finish" << std::endl;
//		m_condNextFile.wait(&m_mutexNextFile);
//	}
//	std::cout << "delete pointers" << std::endl;
//	if(m_input.front() != nullptr){
//		std::cout << "DELETE!" << std::endl;
//		m_input.front()->close();
//		delete m_input.front();
//		m_input.front() = nullptr;
//		fileHandler = nullptr;
//		std::cout << "DONE!" << std::endl;
//	}
//	if(m_output.front() != nullptr){
//		m_output.front()->close();
//		delete m_output.front();
//		m_output.front() = nullptr;
//		mp3Manager = nullptr;
//	}
//	m_mutexNextFile.unlock();
}

void DataManagerConcurrent::finalize() {
	m_mutexNextFile.lock();
	m_condNextFile.sendSignal();
	while (m_queueManager.getQueue().size() != 0) {
		m_condNextFile.wait(&m_mutexNextFile);
	}
	m_condNextFile.sendSignal();
	m_mutexNextFile.unlock();
}

void DataManagerConcurrent::setNoItemsToProcess(bool state) {
	m_mutex.lock();
	m_noItemsToProcess = state;
	m_cond.sendSignal();
	m_mutex.unlock();
}
