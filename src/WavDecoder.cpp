#include "WavDecoder.h"

std::ostream& operator <<(std::ostream &out, const WavHeader& wavHeader) {
	return out << std::left << std::setw(15) << std::setfill(' ') << "chunkId:"
			<< wavHeader.riffChunk.ckID[0] << wavHeader.riffChunk.ckID[1]
			<< wavHeader.riffChunk.ckID[2] << wavHeader.riffChunk.ckID[3]
			<< std::endl << std::left << std::setw(15) << std::setfill(' ')
			<< "chunkSize:" << wavHeader.riffChunk.cksize << std::endl
			<< std::left << std::setw(15) << std::setfill(' ') << "format:"
			<< wavHeader.riffChunk.WAVEID[0] << wavHeader.riffChunk.WAVEID[1]
			<< wavHeader.riffChunk.WAVEID[2] << wavHeader.riffChunk.WAVEID[3]
			<< std::endl <<

			std::left << std::setw(15) << std::setfill(' ') << "subchunk1Id:"
			<< wavHeader.fmtChunk.ckID[0] << wavHeader.fmtChunk.ckID[1]
			<< wavHeader.fmtChunk.ckID[2] << wavHeader.fmtChunk.ckID[3]
			<< std::endl << std::left << std::setw(15) << std::setfill(' ')
			<< "subchunk1Size:" << wavHeader.fmtChunk.cksize << std::endl
			<< std::left << std::setw(15) << std::setfill(' ') << "audioFormat:"
			<< wavHeader.fmtChunk.wFormatTag << std::endl << std::left
			<< std::setw(15) << std::setfill(' ') << "numChannels:"
			<< wavHeader.fmtChunk.nChannels << std::endl << std::left
			<< std::setw(15) << std::setfill(' ') << "sampleRate:"
			<< wavHeader.fmtChunk.nSamplesPerSec << std::endl << std::left
			<< std::setw(15) << std::setfill(' ') << "byteRate:"
			<< wavHeader.fmtChunk.nAvgBytesPerSec << std::endl << std::left
			<< std::setw(15) << std::setfill(' ') << "blockAlign:"
			<< wavHeader.fmtChunk.nBlockAlign << std::endl << std::left
			<< std::setw(15) << std::setfill(' ') << "bitsPerSample:"
			<< wavHeader.fmtChunk.wBitsPerSample << std::endl <<

			std::left << std::setw(15) << std::setfill(' ') << "subchunk2Id:"
			<< wavHeader.dataChunk.ckID[0] << wavHeader.dataChunk.ckID[1]
			<< wavHeader.dataChunk.ckID[2] << wavHeader.dataChunk.ckID[3]
			<< std::endl << std::left << std::setw(15) << std::setfill(' ')
			<< "subchunk2Size:" << wavHeader.dataChunk.cksize << std::endl;
}

WavDecoder::WavDecoder(FILE* file) :
				m_file(file),
				m_headerInit(false)
{
	if (m_file == nullptr)
		throw WavDecoderException(
				"Error opening file: input file does not exist!");
	try {
		decodeHeader();
	} catch (WavDecoderException& e) {
		m_headerInit = false;
		throw e;
	}
}

WavDecoder::~WavDecoder() {
}

WavHeader& WavDecoder::getHeader() {
	return m_header;
}

void WavDecoder::resetDataPosition() {
	fsetpos(m_file, &(m_header.dataChunk.dataPosition));
}

uint16_t WavDecoder::getNChannels() const {
	return m_nChannels;
}

uint32_t WavDecoder::getNBlocks() const {
	return m_nBlocks;
}

// Get the lenght of the data chain (in bytes)
uint32_t WavDecoder::getDataLenght() const {
	return m_header.dataChunk.cksize;
}

uint32_t WavDecoder::getSamplingRate() const {
	return m_samplingRate;
}

uint32_t WavDecoder::getByteRate() const {
	return m_header.fmtChunk.nAvgBytesPerSec;
}

uint16_t WavDecoder::getSampleLength() const {
	return m_sampleLength;
}

int WavDecoder::getWavFormat() const {
	return m_header.fmtChunk.wFormatTag;
}

void WavDecoder::printHeader() {
	std::cout << m_header;
	std::cout << "Total number of blocks is: " << m_nBlocks << std::endl;
	std::cout << "sample length is: " << m_sampleLength << std::endl;
}

int WavDecoder::decodeHeader() {
	try {
		getRiffChunk();
		getFmtChunk();
		getDataChunk();
	} catch (WavDecoderException& e) {
		throw WavDecoderException("Unable to decode WAV header");
	}

	m_nChannels = m_header.fmtChunk.nChannels;
	m_samplingRate = m_header.fmtChunk.nSamplesPerSec;
	m_sampleLength = m_header.fmtChunk.nBlockAlign / m_nChannels;
	m_nBlocks = m_header.dataChunk.cksize / (m_nChannels * m_sampleLength);

//	printHeader();

	m_headerInit = true;

	return 1;
}

bool WavDecoder::checkHeader() {

	bool correctHeader(false);
	if (memcmp(m_header.riffChunk.ckID, m_chunkIDs.riffID, sizeof(uint8_t) * 4)
			!= 0)
	{
		std::cerr << "Not a RIFF file, skipping..." << std::endl;
		correctHeader = false;
	} else if (memcmp(m_header.riffChunk.WAVEID, m_chunkIDs.waveID,
			sizeof(uint8_t) * 4) != 0)
	{
		std::cerr << "Not a WAVE file, skipping..." << std::endl;
		correctHeader = false;
	} else if (memcmp(m_header.dataChunk.ckID, m_chunkIDs.dataID,
			sizeof(uint8_t) * 4) != 0)
	{
		std::cerr << "\"data\" chunk is not defined, skipping..." << std::endl;
		correctHeader = false;
	} else correctHeader = true;

	if (!correctHeader) printHeader();

	return correctHeader;
}

int WavDecoder::getRiffChunk() {

	uint8_t name[] = "RIFF";

	if (!findSubchunkPosition(name))
		throw WavDecoderException("Unable to find RIFF chunk");

	fread(&(m_header.riffChunk), sizeof(m_header.riffChunk), 1, m_file);
	return 1;
}

int WavDecoder::getFmtChunk() {

	uint8_t name[] = "fmt ";

	if (!findSubchunkPosition(name))
		throw WavDecoderException("Unable to find fmt chunk");

	fread(&(m_header.fmtChunk), sizeof(m_header.fmtChunk), 1, m_file);

	return 1;
}

int WavDecoder::getDataChunk() {

	uint8_t name[] = "data";

	if (!findSubchunkPosition(name))
		throw WavDecoderException("Unable to find data chunk");

	fread(&(m_header.dataChunk.ckID), sizeof(uint8_t), 4, m_file);
	fread(&(m_header.dataChunk.cksize), sizeof(uint32_t), 1, m_file);
	fgetpos(m_file, &m_header.dataChunk.dataPosition);

	return 1;
}

int WavDecoder::getFactChunk() {
	uint8_t name[] = "fact";

	if (!findSubchunkPosition(name))
		throw WavDecoderException("Unable to find fact chunk");

	fread(&(m_header.factChunk), sizeof(m_header.factChunk), 1, m_file);

	return 1;
}

bool WavDecoder::findSubchunkPosition(uint8_t* chunkID) {

	bool foundChunk(false);

	fseek(m_file, 0, SEEK_SET);
	uint8_t tmpID1;
	fpos_t pos;
	fpos_t pos2;

	int count(0);
	bool continueSearch(true);
	do {
		fgetpos(m_file, &pos);
		fread(&tmpID1, sizeof(uint8_t), 1, m_file);
		fgetpos(m_file, &pos2);
		if (memcmp(chunkID, &tmpID1, sizeof(tmpID1)) == 0) {
			fsetpos(m_file, &pos);
			uint8_t tmpID[4];
			fread(&(tmpID), sizeof(uint8_t), 4, m_file);
			if (memcmp(chunkID, tmpID, sizeof(tmpID)) == 0) {
				continueSearch = false;
				fsetpos(m_file, &pos);
				foundChunk = true;
			} else fsetpos(m_file, &pos2);
		}
		count++;
		if (count > 144) break;
		if (feof(m_file)) break;
	} while (continueSearch);

	return foundChunk;
}

