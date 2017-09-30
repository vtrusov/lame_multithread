#include "FileHandler.h"

FileHandler::FileHandler(std::string path, bool storeprevblock)
try :
				m_path(path),
				m_file(fopen(m_path.c_str(), "rb")),
				m_wavDecoder(m_file),
				m_prevBuffer(false),
				bufferFloatPtr(nullptr),
				bufferIntPtr(nullptr),
				sizeOfBuff(0),
				m_storePrevBlock(storeprevblock)
{
	m_validFile = true;
}
catch (WavDecoderException& e) {
	m_validFile = false;
	close();
	throw FileHandlerException("Invalid file header");
}
catch (...) {
	m_validFile = false;
	throw;
}

FileHandler::~FileHandler() {
	if (m_file != nullptr) close();
}

int FileHandler::readBlock(float* buffer, int size) {
	int nRead = fread(buffer, sizeof(float), size, m_file);
	if (m_storePrevBlock) {
		if (nRead > 0) {
			// arbitrary minimal number of frames for successful "glue" of frames encoded by different encoder instances
			sizeOfBuff = 4 * size / 10;
			if (sizeOfBuff > nRead) sizeOfBuff = nRead;
//			memcpy(&bufferFloat[0], &buffer[nRead - sizeOfBuff],
//					sizeof(float) * sizeOfBuff);
			bufferFloatPtr = &buffer[nRead - sizeOfBuff];
			m_prevBuffer = true;
		}
	}
	return nRead;
}

int FileHandler::readBlock(short int* buffer, int size) {
	int nRead = fread(buffer, sizeof(short int), size, m_file);
	if (m_storePrevBlock) {
		if (nRead > 0) {
			// arbitrary minimal number of frames for successful "glue" of frames encoded by different encoder instances
			sizeOfBuff = 4 * size / 10;
			if (sizeOfBuff > nRead) sizeOfBuff = nRead;
//			memcpy(&bufferInt[0], &buffer[nRead - sizeOfBuff],
//					sizeof(short int) * sizeOfBuff);
			bufferIntPtr = &buffer[nRead - sizeOfBuff];
			m_prevBuffer = true;
		}
	}
	return nRead;
}

int FileHandler::getPrevBuffer(float* buffer) {
	if (m_prevBuffer && m_storePrevBlock) {
		memcpy(&buffer[0], &bufferFloatPtr[0], sizeof(float) * sizeOfBuff);
	}
	return sizeOfBuff;
}

int FileHandler::getPrevBuffer(short int* buffer) {
	if (m_prevBuffer && m_storePrevBlock) {
		memcpy(&buffer[0], &bufferIntPtr[0], sizeof(short int) * sizeOfBuff);
	}
	return sizeOfBuff;
}

WavDecoder FileHandler::getWavDecoder() {
	return m_wavDecoder;
}

void FileHandler::resetFilePosition() {
	m_wavDecoder.resetDataPosition();
}

void FileHandler::close() {
	if (m_file) {
		fclose (m_file);
		m_file = nullptr;
	}
}

bool FileHandler::endOfFile() const {
	return feof(m_file);
}

bool FileHandler::isValidWavFile() const {
	return m_validFile;
}

std::string FileHandler::getFilePath() {
	return m_path;
}
