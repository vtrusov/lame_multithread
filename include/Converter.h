#pragma once

#include "FileHandler.h"
#include "LameEncoder.h"
#include "BufferManager.h"
#include <cassert>

/*
 * Interface implementation
 */
class ConverterInterface {
public:
	virtual ~ConverterInterface() {}
	virtual void init(FileHandler*) = 0;
	virtual void bufferInit() = 0;
	virtual int readBlock() = 0;
	virtual int convertBlock(int nBlocks) = 0;
	virtual int finalize() = 0;
	virtual unsigned char* getMP3Buffer() const = 0;
	virtual void checkBuffers() = 0;
	virtual void resizeBuffers() = 0;
	virtual int flushBuffer(int nBlocks) = 0;
	virtual bool getFinalized() = 0;
};


/*
 * Converter class, uses LameEncoder to convert WAV buffer to MP3.
 * The data stored internally in buffers, and passed to the encoder as pointers to the data array.
 * Input data is taken from the FileHandler* instance.
 */
template<typename pcmBufferType>
class Converter: public ConverterInterface {
public:

	// Base class
	typedef ConverterInterface super;

	// Constructor
	Converter() :
					m_fileHandler(nullptr),
					m_pcmBuffer(nullptr),
					m_mp3Buffer(nullptr),
					m_finalized(false)
	{}

	// Virtual destructor
	virtual ~Converter() {
		if (m_pcmBuffer != nullptr) {
			delete m_pcmBuffer;
			m_pcmBuffer = nullptr;
		}
		if (m_mp3Buffer != nullptr) {
			delete m_mp3Buffer;
			m_mp3Buffer = nullptr;
		}
	}

	// Forbid to copy the objects
	Converter(const Converter<pcmBufferType>& other) = delete;

	// Store pointer to the FileHandler object and initialize LameEncoder
	void init(FileHandler* fileHandler) {
		m_fileHandler = fileHandler;
		m_lameEncoder.initLame(m_fileHandler->getWavDecoder());
		m_finalized = false;
		if (m_pcmBuffer == nullptr) m_pcmBuffer = new pcmBufferType();
		if (m_mp3Buffer == nullptr) m_mp3Buffer = new BufferChar();
	}

	// Allocate space for internal buffers
	void bufferInit() {
		m_pcmBuffer->bufferInit(m_lameEncoder.getPCMBufferSize());
		m_mp3Buffer->bufferInit(m_lameEncoder.getMP3BufferSize());
	}

	// Read block from input WAV file
	int readBlock() {
		return m_fileHandler->readBlock(m_pcmBuffer->getBuffer(),
				m_pcmBuffer->getBufferSize());
	}

	// Convert PCM data, stored in internal buffer
	int convertBlock(int nBlocks) {
		int nWriteBlocks(0);
		if (nBlocks == 0) nWriteBlocks = finalize();
		else nWriteBlocks = m_lameEncoder.encodeBuffer(m_pcmBuffer->getBuffer(),
				nBlocks, m_mp3Buffer->getBuffer());
		return nWriteBlocks;
	}

	// Flush lame buffers
	int flushBuffer(int nBlocks) {
		unsigned char* newBufferPos = m_mp3Buffer->getBuffer() + nBlocks;
		return m_lameEncoder.lameFinalizeNoGap(newBufferPos);;
	}

	// flush lame buffers and get store info in mp3 buffer to finalize mp3 file
	int finalize() {
		m_finalized = true;
		return m_lameEncoder.lameFinalize(m_mp3Buffer->getBuffer());
	}

	// get pointer to mp3 buffer
	unsigned char* getMP3Buffer() const {
		return m_mp3Buffer->getBuffer();
	}

	// checks whether finalize() function was called.
	bool getFinalized() {
		return m_finalized;
	}

protected:

	// check internal buffers
	void checkBuffers() {
		if ((m_pcmBuffer == nullptr) || (m_mp3Buffer == nullptr)) {
			std::cerr << "Converter: Buffers are not defined!" << std::endl;
			assert(false);
		}
		if ((m_mp3Buffer->getBufferSize() != m_lameEncoder.getMP3BufferSize())
				|| (m_pcmBuffer->getBufferSize()
						!= m_lameEncoder.getPCMBufferSize())) resizeBuffers();
	}

	// update size of the buffers
	void resizeBuffers() {
		m_mp3Buffer->resizeBuffer(m_lameEncoder.getMP3BufferSize());
		m_pcmBuffer->resizeBuffer(m_lameEncoder.getPCMBufferSize());
	}

	FileHandler* m_fileHandler; 	// pointer to FileHandler object, which holds input WAV stream
	LameEncoder m_lameEncoder;		// LameEncoder object, which performs conversion of PCM data to MP3 data

	pcmBufferType* m_pcmBuffer;		// buffer with PCM data
	BufferChar* m_mp3Buffer;		// buffer with MP3 data

	bool m_finalized;				// variable to check whether finalize() routine was called
};

// Class to convert uncompressed PCM to MP3 (PCM mode is 1)
class ConverterPCM: public Converter<BufferInt> {
};

// Class to convert compressed PCM to MP3 (PCM mode is 3)
class ConverterIEEE: public Converter<BufferFloat> {
};

