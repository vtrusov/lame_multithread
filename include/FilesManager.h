#pragma once

#include "FileHandler.h"

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#ifdef __GNUC__
#include <dirent.h>
#else
#include <windows.h>
#endif

/*
 * Class to hold list of WAV files to process
 */
class FilesManager {
public:

	FilesManager(std::string path);
	~FilesManager();

	// fill the list of wav files
	void collectFiles();

	// get the path to the next file to process
	std::string getNextFile();

	// generate name of mp3 file
	std::string generateMP3Name(std::string wavFile);

	// erase invalid files from the list
	void eraseNonWav();

	// print the list of files
	void listFiles();

	// check whether the specified file has WAV attributes
	bool isWavFile(std::string file);

	// get the number of files
	int getNFiles();

private:

	// append filename to the directory path
	std::string composePath(std::string file);

	std::vector<std::string>::iterator m_fileIterator;	// iterator through the list of files
	std::string m_path;									// path to the directory with wav files
	std::vector<std::string> m_files;					// list of wav files to process
};

