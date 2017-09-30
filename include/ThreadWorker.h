#pragma once

#include "Converter.h"
#include "DataManager.h"
#include "MutexManager.h"
#include "QueueManager.h"

#include "FileHandler.h"
#include "MP3Manager.h"

#include "Thread.h"

/*
 * the class to perform threaded data processing
 */
class ThreadWorker: public Thread {
public:

	typedef Thread super;
	friend class DataManager;

	// constructor
	ThreadWorker(DataManager* dataManager) :
					m_converter(nullptr),
					m_dataManager(dataManager),
					m_mp3Manager(nullptr)
	{
	}

	// destructor
	~ThreadWorker() {}


	// function which performs data processing
	void *run() final {

		for (;;) {
			FileHandler* input = nullptr;
			int status = m_dataManager->getFile(input, m_mp3Manager);

			if (status == 0){
				m_dataManager->finalize();
				m_dataManager->deletePointers(input, m_mp3Manager);
				break;
			}

			if (input == nullptr){
				m_dataManager->finalize();
				m_dataManager->deletePointers(input, m_mp3Manager);
				continue;
			}

			int PCMmode = input->getWavDecoder().getWavFormat();

			m_converter = m_dataManager->generateConverter(PCMmode);
			m_converter->init(input);
			m_converter->bufferInit();

			int readInfo;
			do {
				readInfo = readData();
				int writeInfo = processData(readInfo);
				writeData(writeInfo);
			} while (!(m_mp3Manager->isFileFinalized()));

			delete m_converter;
			m_converter = nullptr;
			m_dataManager->finalize();
			m_dataManager->deletePointers(input, m_mp3Manager);

		}
		return NULL;
	}
	;

private:

	// read data from the input stream
	int readData() {
		return m_dataManager->readData(this);
	}

	// convert data
	int processData(int nReads) {
		int nBlocksWrite = m_converter->convertBlock(nReads);
		return nBlocksWrite;
	}

	// write converted data
	int writeData(int nBlocksWrite) {
		m_dataManager->writeData(this, m_mp3Manager, nBlocksWrite);
		return 0;
	}

	ConverterInterface* m_converter;	// pointer to the converter object
	DataManager* m_dataManager;			// pointer to the data manager object
	MP3Manager* m_mp3Manager;			// pointer to the output stream
};

