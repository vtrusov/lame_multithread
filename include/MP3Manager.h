#pragma once

#include <stdio.h>
#include <string>

/*
 * class to hold output mp3 file
 */
class MP3Manager {
public:

	// constructor
	MP3Manager(std::string path) :
					m_path(path),
					m_file(fopen(m_path.c_str(), "wb")),
					m_isFileFinalized(false)
	{}

	// destructor
	~MP3Manager() {
		if (m_file != nullptr) close();
	}

	// write nBlocks to mp3 file
	void write(unsigned char* buffer, int nBlocks) {
		fwrite(buffer, nBlocks, 1, m_file);
	}

	// check whether file is finalized
	bool isFileFinalized() const {
		return m_isFileFinalized;
	}

	// set file finalized
	void setFinalized(bool finalized = true) {
		m_isFileFinalized = finalized;
	}

	// close output file
	void close() {
		fclose(m_file);
		m_file = nullptr;
	}

	// get filename
	std::string getFilename() const {
		return m_path;
	}

private:

	std::string m_path; 	// path to the file
	FILE* m_file;			// output stream

	bool m_isFileFinalized;	// set whether file is finalized

};

