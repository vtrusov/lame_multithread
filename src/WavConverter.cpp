// wav test samples http://www.music.helsinki.fi/tmt/opetus/uusmedia/esim/index-e.html

#include "ThreadFactory.h"
#include "ThreadWorker.h"

#include "FilesManager.h"

#include <string>
#include <iostream>

#include <chrono>
#include "ThreadManager.h"

int main(int argc, char** argv)
{
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

    if (argc != 2)
	{
		std::cerr<< "Usage: " << argv[0] << " <directory with wav files>\n" << std::endl;
		return (-1);
	}
	std::string input(argv[1]);


	FilesManager filesManager(input);
	filesManager.collectFiles();

	// at this stage select mode of data processing, concurrent or parallel
	// concurrent mode is slower since it includes prebuffer encoding stage, which means larger dataset should be encoded
	// parallel mode cannot benefit from parallel processing in case of a small amount of wav files
	// based on the number of files in the directory, the mode is selected
	const bool isConcurrent = (filesManager.getNFiles() <= ThreadManager::getNCores());

	// create ThreadManager
	ThreadManager* threadManager;
	if(isConcurrent)
		threadManager = new ThreadManagerConcurrent();
	else
		threadManager = new ThreadManagerParallel();

	ThreadFactory* threadFactory;
	threadFactory = new ThreadWorkerFactory();

	threadManager->setNoItemsToProcess(false);

	std::string file;
	do{
		file = filesManager.getNextFile();
		if(file == "") {
			threadManager->setNoItemsToProcess(true);
			break;
		}
		std::cout<<"add following file to the queue: " << file << std::endl;
		threadManager->addFile(file);
	} while (file != "");

	// perform data processing
	threadManager->init(threadFactory);
	threadManager->runThreads();
	threadManager->joinThreads();
	threadManager->terminate();

	delete threadManager;
	delete threadFactory;

	std::chrono::high_resolution_clock::time_point stop_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( stop_time - start_time ).count();
	std::cout << "Execution time is "<< duration/1000. << "s" << std::endl;

}

