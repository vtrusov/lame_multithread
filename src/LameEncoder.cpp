#include "LameEncoder.h"

LameEncoder::LameEncoder() :
				m_lame(nullptr),
				m_lameVer(0),
				m_sizePCMBuffer(0),
				m_sizeMP3Buffer(0),
				m_pcmMode(0)
{
}

LameEncoder::~LameEncoder() {
	lame_close(m_lame);
}

void LameEncoder::initLame(const WavDecoder& wavDecoder) {
	if (m_lame != nullptr) lame_close(m_lame);

	m_lame = lame_init();

	// set lame encoder parameters
	lame_set_num_channels(m_lame, wavDecoder.getNChannels());
	lame_set_in_samplerate(m_lame, wavDecoder.getSamplingRate());
	if (wavDecoder.getNChannels() == 2) lame_set_mode(m_lame, STEREO);
	else if (wavDecoder.getNChannels() == 1) lame_set_mode(m_lame, MONO);
	lame_set_disable_reservoir(m_lame, 1);

	lame_set_quality(m_lame, 2);

	int lame_init_code = lame_init_params(m_lame);

	if (lame_init_code < 0) {
		std::cerr << "Error while initializing lame! Error code: "
				<< lame_init_code << std::endl;
		lame_close(m_lame);
		m_lame = nullptr;
		return;
	}

	/* version  0=MPEG-2  1=MPEG-1  (2=MPEG-2.5)     */
	m_lameVer = lame_get_version(m_lame);

	// set size of the PCM buffer. The numbers are taken BladeMP3EncDLL.c example (which is included in lame lib) and are exactly the size of the encoded frame
	if (0 == m_lameVer) m_sizePCMBuffer = 576 * lame_get_num_channels(m_lame);
	else m_sizePCMBuffer = 1152 * lame_get_num_channels(m_lame);
	m_sizePCMBuffer = m_sizePCMBuffer * 10;

	// set size of the mp3 buffer
	m_sizeMP3Buffer = 1.25 * m_sizePCMBuffer / lame_get_num_channels(m_lame)
			+ 7200;

	m_pcmMode = wavDecoder.getWavFormat();
}

void LameEncoder::close() {
	lame_close(m_lame);
}

int LameEncoder::getMP3BufferSize() const {
	return m_sizeMP3Buffer;
}

int LameEncoder::getPCMBufferSize() const {
	return m_sizePCMBuffer;
}

int LameEncoder::getPCMmode() const {
	return m_pcmMode;
}

//encode buffer. Outputs number of bytes in mp3Buffer
int LameEncoder::encodeBuffer(float* wavBuffer, int nSamples,
		unsigned char* mp3Buffer)
{
	int output(0);
	if (1 == lame_get_num_channels(m_lame)) output =
			lame_encode_buffer_ieee_float(m_lame, wavBuffer, wavBuffer,
					nSamples / lame_get_num_channels(m_lame), mp3Buffer,
					m_sizeMP3Buffer);
	else output = lame_encode_buffer_interleaved_ieee_float(m_lame, wavBuffer,
			nSamples / lame_get_num_channels(m_lame), mp3Buffer,
			m_sizeMP3Buffer);
	return output;
}

//encode buffer. Outputs number of bytes in mp3Buffer
int LameEncoder::encodeBuffer(short int* wavBuffer, int nSamples,
		unsigned char* mp3Buffer)
{
	int output(0);
	if (1 == lame_get_num_channels(m_lame)) output = lame_encode_buffer(m_lame,
			wavBuffer, wavBuffer, nSamples / lame_get_num_channels(m_lame),
			mp3Buffer, m_sizeMP3Buffer);
	else output = lame_encode_buffer_interleaved(m_lame, wavBuffer,
			nSamples / lame_get_num_channels(m_lame), mp3Buffer,
			m_sizeMP3Buffer);
	return output;
}

int LameEncoder::lameFinalize(unsigned char* mp3Buffer) {
	return lame_encode_flush(m_lame, mp3Buffer, m_sizeMP3Buffer);
}

int LameEncoder::lameFinalizeNoGap(unsigned char* mp3Buffer) {
	return lame_encode_flush_nogap(m_lame, mp3Buffer, m_sizeMP3Buffer);
}

int LameEncoder::getNChannels() {
	return lame_get_num_channels(m_lame);
}
