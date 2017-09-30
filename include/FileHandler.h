#pragma once

#include "WavDecoder.h"

#include <stdio.h>
#include <string>
#include <cstring>
#include <exception>

class FileHandlerException: public std::exception {
	std::string m_message;
public:
	FileHandlerException(std::string id = "Error occurred while reading file") :
					std::exception(),
					m_message(id)
	{
	}
	;

	const char* what() const throw() {
		return m_message.c_str();
	}
};

/*
 * Class to handle WAV input file, and to hold its header information
 */
class FileHandler {
public:

	FileHandler(std::string path, bool storeprevblock = false);
	~FileHandler();

	// forbid copy of the object
	FileHandler(const FileHandler& other) = delete;

	// read float data block
	int readBlock(float* buffer, int size) ;

	// read short int data block
	int readBlock(short int* buffer, int size) ;

	// get buffered data from previous read step
	int getPrevBuffer(float* buffer) ;

	// get buffered data from previous read step
	int getPrevBuffer(short int* buffer) ;

	// get WavDecoder object
	WavDecoder getWavDecoder() ;

	// set read position to the beginning of the PCM data
	void resetFilePosition() ;

	// close input file
	void close() ;

	// check whether the end of file was reached
	bool endOfFile() const ;

	// check whether input file is valid
	bool isValidWavFile() const ;

	// get path to the input file
	std::string getFilePath() ;

private:

	std::string m_path;			// variable to hold path to the input file
	FILE* m_file;				// input stream object
	WavDecoder m_wavDecoder;	// wav header decoder object

	bool m_prevBuffer;			// set true in case previous data block has been stored
	float* bufferFloatPtr;		// pointer to previous buffer
	short int* bufferIntPtr;	// pointer to previous buffer
	int sizeOfBuff;				// size of the previous buffer

	bool m_validFile;			// set false if wav file has invalid header
	bool m_storePrevBlock;		// set true in case previous data read should be stored
};

