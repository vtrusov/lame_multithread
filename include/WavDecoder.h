// TODO: check compatibility with non-intel processors (big-endian). WAV files are little-endian as well as intel processors https://stackoverflow.com/questions/3692465/why-are-an-integers-bytes-stored-backwards-does-this-apply-to-headers-only

//http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <exception>
#include <iomanip>
#include <cstdint>
#include <wchar.h>

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007

class WavDecoderException: public std::exception {
	std::string m_message;
public:
	WavDecoderException(std::string id =
			"Error occurred while decoding WAV header") :
					std::exception(),
					m_message(id)
	{
	}

	const char* what() const throw() {
		return m_message.c_str();
	}
};

typedef struct chunkIDs_s {
	// IDs stored in char[5] arrays to handle trailing 0 character at the end of char sequence
//	const uint8_t riffID[5] = reinterpret_cast<const unsigned char *>("RIFF");
	//const uint8_t waveID[5] = reinterpret_cast<const unsigned char *>("WAVE");
	//const uint8_t fmtID[5] = reinterpret_cast<const unsigned char *>("fmt ");
	//const uint8_t dataID[5] = reinterpret_cast<const unsigned char *>("data");
	const uint8_t riffID[4] = { 'R', 'I', 'F', 'F' };
	const uint8_t waveID[4] = { 'W', 'A', 'V', 'E' };
	const uint8_t fmtID[4] = { 'f', 'm', 't', ' ' };
	const uint8_t dataID[4] = { 'd', 'a', 't', 'a' };
} chunkIDs;

typedef struct RiffChunk_s {
	uint8_t ckID[4]; 			// Chunk ID: "RIFF"
	uint32_t cksize; 			// Chunk size: 4 + Wave chunks containing format information and sampled data
	uint8_t WAVEID[4];			// WAVE ID: "WAVE"
} RiffChunk; // RIFF chunk with WAVE identifier

typedef struct FmtChunk_s {
	uint8_t ckID[4];			// Chunk ID: "fmt"
	uint32_t cksize;			// Chunk size: 16, 18 or 40
	uint16_t wFormatTag;		// Format code
	uint16_t nChannels;			// Number of interleaved channels (Nc)
	uint32_t nSamplesPerSec;	// Sampling rate (blocks per second) (F)
	uint32_t nAvgBytesPerSec;	// Data rate (F*M*Nc,  M is the lenght of each sample in bytes)
	uint16_t nBlockAlign;    	// Data block size (bytes) (M*Nc)
	uint16_t wBitsPerSample; 	// Bits per sample (8*M)
} FmtChunk; // the chunk which specifies the format of the data

typedef struct FactChunk_s {
	uint8_t ckID[4];			// Chunk ID: "fact"
	uint32_t cksize;			// Chunk size
	uint32_t dwSampleLength; 	// Number of samples (per channel)
} FactChunk; // the optional chunk which is contained in non-PCM encoded files

typedef struct {
	uint8_t ckID[5];
	uint32_t cksize;
	fpos_t dataPosition; // variable to hold the position of the WAVE data
} DataChunk; // the chunk which contains the sampled data

typedef struct WavHeader_s {
	RiffChunk riffChunk;
	FmtChunk fmtChunk;
	FactChunk factChunk;
	DataChunk dataChunk;
} WavHeader;

/*
 * Class to handle wav header
 */
class WavDecoder {
public:

	WavDecoder(FILE* file);
	~WavDecoder();

	// check whether wav header is correct
	bool checkHeader();

	// get wav header
	WavHeader& getHeader();

	// set input stream position to the beginning of PCM data
	void resetDataPosition();

	// get number of channels
	uint16_t getNChannels() const;

	// get number of PCM blocks
	uint32_t getNBlocks() const;

	// get the lenght of the data chain (in bytes)
	uint32_t getDataLenght() const;

	// get sampling rate
	uint32_t getSamplingRate() const;

	//get byte rate
	uint32_t getByteRate() const;

	// get the lenght of sample
	uint16_t getSampleLength() const;

	// get PCM format
	int getWavFormat() const;

	// print WAV header
	void printHeader();

	// read wav header
	int decodeHeader();

private:

	// find and decode "RIFF" chunk
	int getRiffChunk();

	// find and decode "fmt " chunk
	int getFmtChunk();

	// find and decode "fact" chunk
	int getFactChunk();

	// find and decode "data" chunk
	int getDataChunk();

	// Function to locate specified chunk.
	// As WAVE specification allows to place subchunks in manual order, subchunks are looked up
	// by its ID ("RIFF", "fmt ", "data")
	bool findSubchunkPosition(uint8_t* chunkID);

	WavHeader m_header;			// wav header

	FILE* m_file;				// input stream

	uint16_t m_nChannels;		// number of channels
	uint32_t m_nBlocks;			// number of blocks
	uint32_t m_samplingRate;	// sampling rate
	uint16_t m_sampleLength;	// lenght of sample

	bool m_headerInit;			// sets whether header was decoded

	chunkIDs m_chunkIDs;		// default chunk IDs
};
