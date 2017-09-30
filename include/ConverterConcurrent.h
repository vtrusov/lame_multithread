#pragma once

#include "Converter.h"

#include "FileHandler.h"
#include "LameEncoder.h"
#include "BufferManager.h"
#include <cassert>

/*
 * Concurrent converter class, derived from Converter
 * The purpose of this class is to hold two kinds of PCM data: actual frame to be converted and previous frame.
 * In case of a concurrent PCM to MP3 conversion, the PCM data sequence in each thread (instantiations of this class)
 * passed to the lame encoder will be broken.
 * Since lame frame encode process relies on previous encoded frames, the next procedure is introduced:
 * 		1. prebuffered data is encoded to "prepare" lame
 * 		2. actual data is encoded
 * In case the step 1 is skipped, the beginning of encoded actual frame will be distorted.
 * This technique allows to glue frames encoded by multiple instantiations of the lame encoder.
 */
template<typename pcmBufferType>
class ConverterConcurrent: public Converter<pcmBufferType>{
public:

	// Base class
	typedef Converter<pcmBufferType> super;

	// Constructor
	ConverterConcurrent(bool preBuffer = true) : super(), m_pcmPreBuffer(nullptr), m_sizePreBuffer(0), m_usePreBuffer(preBuffer) {};

	// Destructor
	~ConverterConcurrent() {
		if(m_pcmPreBuffer != nullptr){
			delete m_pcmPreBuffer;
			m_pcmPreBuffer = nullptr;
		}
	};

	// Forbid copy
	ConverterConcurrent(const ConverterConcurrent<pcmBufferType>& other) = delete;

	// initialization of lame
	void init(FileHandler* fileHandler){
		super::init(fileHandler);
		if(m_pcmPreBuffer == nullptr) m_pcmPreBuffer = new pcmBufferType();
	}

	// initialization of buffers
	void bufferInit(){
		super::bufferInit();
		m_pcmPreBuffer->bufferInit(super::m_lameEncoder.getPCMBufferSize());
	};

	// read previous and actual PCM data blocks
	int readBlock() {
		m_sizePreBuffer = preBufferData();
		return super::m_fileHandler->readBlock(super::m_pcmBuffer->getBuffer(), super::m_pcmBuffer->getBufferSize());
	};

	// convert previous data block and actual data block
	int convertBlock(int nBlocks) {
		convertBuffer(m_sizePreBuffer);
		int nWriteBlocks(0);
		if(nBlocks == 0) nWriteBlocks = super::finalize();
		else {
			nWriteBlocks = super::m_lameEncoder.encodeBuffer(super::m_pcmBuffer->getBuffer(), nBlocks, super::m_mp3Buffer->getBuffer());
		}
		return nWriteBlocks;
	};

protected:

	// get previous PCM data and store if in preBuffer
	int preBufferData() {
		if(!m_usePreBuffer) return 0;
		return super::m_fileHandler->getPrevBuffer(m_pcmPreBuffer->getBuffer());
	}

	// convert prebuffered data to prepare lame for decoding of actual data
	int convertBuffer(int nBlocks) {
		if(!m_usePreBuffer) return 0;
		int nWriteBlocks(0);
		if(nBlocks > 0){
			nWriteBlocks = super::m_lameEncoder.encodeBuffer(m_pcmPreBuffer->getBuffer(), nBlocks, super::m_mp3Buffer->getBuffer());
		}
		return nWriteBlocks;
	}

	// resize buffers
	void resizeBuffers(){
		super::m_mp3Buffer->resizeBuffer(super::m_lameEncoder.getMP3BufferSize());
		super::m_pcmBuffer->resizeBuffer(super::m_lameEncoder.getPCMBufferSize());
		m_pcmPreBuffer->resizeBuffer(super::m_lameEncoder.getPCMBufferSize());
	}


	pcmBufferType* m_pcmPreBuffer; 	// buffer to store previous PCM data
	int m_sizePreBuffer;			// size of the buffer

	bool m_usePreBuffer;			// set whether prebuffered data shall be converted
};

// Class to convert uncompressed PCM to MP3 (PCM mode is 1)
class ConverterConcurrentPCM: public ConverterConcurrent<BufferInt>{
public:
	ConverterConcurrentPCM(bool preBuffer = true): ConverterConcurrent<BufferInt>(preBuffer) {};
};

// Class to convert compressed PCM to MP3 (PCM mode is 3)
class ConverterConcurrentIEEE: public ConverterConcurrent<BufferFloat>{
public:
	ConverterConcurrentIEEE(bool preBuffer = true): ConverterConcurrent<BufferFloat>(preBuffer) {};
};


