#pragma once
#include "WavDecoder.h"

#include <type_traits>
#include "lame.h"

/*
 * lame wrapper class
 */
class LameEncoder {
public:

	LameEncoder();
	~LameEncoder();

	// close lame encoder
	void close();

	// initialize lame encoder
	void initLame(const WavDecoder& m_wavDecoder);

	// get size of the MP3 buffer
	int getMP3BufferSize() const;

	// get size of the PCM buffer
	int getPCMBufferSize() const;

	// get pcm mode of input file
	int getPCMmode() const;

	//encode float buffer. Outputs number of bytes in mp3Buffer
	int encodeBuffer(float* wavBuffer, int nSamples, unsigned char* mp3Buffer);

	//encode short int buffer. Outputs number of bytes in mp3Buffer
	int encodeBuffer(short int* wavBuffer, int nSamples,
			unsigned char* mp3Buffer);

	// call lame_encode_flush function
	int lameFinalize(unsigned char* mp3Buffer);

	// call lame_encode_flush_nogap function
	int lameFinalizeNoGap(unsigned char* mp3Buffer);

	// get number of channels
	int getNChannels();

private:

	lame_global_flags *m_lame;	// lame encoder
	int m_lameVer;				// lame version

	int m_sizePCMBuffer;		// size of the PCM buffer
	int m_sizeMP3Buffer;		// size of the MP3 buffer
	int m_pcmMode;				// PCM format code
};

