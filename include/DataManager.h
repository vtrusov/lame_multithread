#pragma once

#include "FileHandler.h"
#include "MP3Manager.h"
#include "Converter.h"
#include "QueueManager.h"
#include "MutexManager.h"

#include "Mutex.h"
#include <string>
#include <list>
#include <vector>
#include "ConverterConcurrent.h"

class ThreadWorker;

/*
 * Data Manager interface
 * Derived classes are used to manage read/convert/write process inside threads
 */
class DataManager {
public:
	DataManager() {};
	virtual ~DataManager() {};
	virtual void addFile(std::string) = 0;
	virtual int getFile(FileHandler* &fileHandler, MP3Manager* &mp3Manager) = 0;
	virtual void nextFile() = 0;
	virtual int readData(ThreadWorker* threadWorker) = 0;
	virtual int writeData(ThreadWorker* threadWorker, MP3Manager* mp3Manager, int nBlocksWrite) = 0;
	virtual void initThread(uint64_t id) = 0;
	virtual ConverterInterface* generateConverter(int pcmMode) = 0;
	virtual void setNoItemsToProcess(bool state) = 0;
	virtual void deletePointers(FileHandler* &fileHandler, MP3Manager* &mp3Manager) = 0;
	virtual void finalize() = 0;

protected:
	// helper function to give access to the private member (m_converter) of the ThreadWorker class for derived classes
    ConverterInterface* getConverter(ThreadWorker* threadWorker) ;
    // get id of the calling ThreadWorker object
    uint64_t getID(ThreadWorker* threadWorker) ;
};


/*
 * Data manager class to manage parallel data processing.
 * Each wav file is read/converted/stored by a single thread.
 */
class DataManagerParallel: public DataManager{
public:

	// Base class
	typedef DataManager super;

	DataManagerParallel() ;
	~DataManagerParallel() ;

	// add file to the conversion list
	void addFile(std::string file) ;

	// get file to process from the list. The function replaces pointers to the FileHandler and MP3Manager objects
	int getFile(FileHandler* &fileHandler, MP3Manager* &mp3Manager) ;

	// proceed to the next file
	void nextFile() ;

	// empty function
	void initThread(uint64_t id) ;

	// read block of data
	int readData(ThreadWorker* threadWorker)  ;

	// write converted mp3 data
	int writeData(ThreadWorker* threadWorker, MP3Manager* mp3Manager, int nBlocksWrite) ;

	// create converter object; the choice depends on the PCM mode of input PCM file
	ConverterInterface* generateConverter(int pcmMode) ;

	// set true in case of no more files to add left
	void setNoItemsToProcess(bool state) ;

	// manage resources
	void deletePointers(FileHandler* &fileHandler, MP3Manager* &mp3Manager) ;

	// empty function
	void finalize() {};

private:

	Mutex m_mutex;	// mutex
	Cond m_cond;	// condition variable

	std::list<FileHandler*> m_input;	// list to hold FileHandler object with WAV files
	std::list<MP3Manager*> m_output;	// list to hold output files

	bool m_noItemsToProcess;			// sets true in case no more files will be added to the list
};

/*
 * Data manager class to manage concurrent pcm data conversion.
 * Each WAV file is processed by all threads available (in concurrent mode).
 * PCM data reading order is stored internally in the access queue and later used to restore the write access order of threads.
 * Each thread has a corresponding write mutex, which is locked when thread reads the next data block,
 * and unlocked when all previous data blocks has been processed and stored.
 */
class DataManagerConcurrent: public DataManager{
public:

	// Base class
	typedef DataManager super;

	DataManagerConcurrent(bool prebuffer = true) ;
	~DataManagerConcurrent() ;

	// add file to the conversion list
	void addFile(std::string file) ;

	// get file to process from the list. The function replaces pointers to the FileHandler and MP3Manager objects
	int getFile(FileHandler* &fileHandler, MP3Manager* &mp3Manager) ;

	// proceed to the next file
	void nextFile() ;

	// init thread -- add its ID to the mutex manager
	void initThread(uint64_t id) ;

	// read block of data
	int readData(ThreadWorker* threadWorker)  ;

	// write converted data -- the thread will wait in this routine until all previous threads finish writing
	int writeData(ThreadWorker* threadWorker, MP3Manager* mp3Manager, int nBlocksWrite) ;

	// create converter object; the choice depends on the PCM mode of input PCM file
	ConverterInterface* generateConverter(int pcmMode) ;

	// set true in case of no more files to add left
	void setNoItemsToProcess(bool state) ;

	// manage resources
	void deletePointers(FileHandler* &fileHandler, MP3Manager* &mp3Manager) ;

	// function to wait all threads to finish processing before proceeding to the next file
	void finalize() ;

private:

	bool m_prebufferPCM;	// sets whether prebuffered data (previous frames) shall be converted

	Mutex m_mutexNextFile;	// mutex to lock proceeding to the next file
	Cond m_condNextFile;	// condition variable for the m_mutexNextFile mutex
	Mutex m_mutex;			// mutex
	Cond m_cond;			// condition variable

	std::list<FileHandler*> m_input;	// list to hold FileHandler object with WAV files
	std::list<MP3Manager*> m_output;	// list to hold output files

	QueueManager m_queueManager;	// queue manager which stores access order to the data
	MutexManager m_mutexManager;	// mutex manager which manages write access to the MP3 file

	bool m_noItemsToProcess;		// sets true in case no more files will be added to the list
};
