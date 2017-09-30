#pragma once

/*
 * Data buffer interface
 */
template<typename T>
class BufferManagerInterface {
public:
	virtual ~BufferManagerInterface() {}
	virtual void bufferInit(int bufferSize) = 0;
	virtual void clear() = 0;
	virtual T* getBuffer() const = 0;
	virtual int getBufferSize() const = 0;
	virtual void resizeBuffer(int bufferSize) = 0;
};

/*
 * Data buffer implementation.
 * Template parameter T allows to create and manager data buffers of different type
 * The class is used to hold and forward pointer to the data array and its size.
 */
template<typename T>
class BufferManager: public BufferManagerInterface<T> {
public:

	// Base class
	typedef BufferManagerInterface<T> super;

	// Constructor
	BufferManager() :
					m_bufferSize(0),
					m_buffer(nullptr)
	{}

	// Destructor
	~BufferManager() {
		clear();
	}

	// Forbid copy of the object
	BufferManager(const BufferManager& other) = delete;
	void operator=(const BufferManager& other) = delete;

	// allocate buffer
	void bufferInit(int bufferSize) {
		m_bufferSize = bufferSize;
		if (m_buffer != nullptr) clear();
		m_buffer = new T[m_bufferSize];
	}

	// clear buffer
	void clear() {
		if (m_buffer != nullptr) {
			delete[] m_buffer;
			m_buffer = nullptr;
		}
	}

	// get pointer to the buffer
	T* getBuffer() const {
		return m_buffer;
	}

	// get size of the buffer
	int getBufferSize() const {
		return m_bufferSize;
	}

	// reallocate buffer
	void resizeBuffer(int bufferSize) {
		clear();
		bufferInit(m_bufferSize);
	}

private:

	int m_bufferSize; 	// variable to hold the size of the buffer
	T* m_buffer;		// buffer

};

/*
 * Buffer with short int entries
 */
class BufferInt: public BufferManager<short int> {
//public: BufferInt():BufferManager<short int>() {};
};

/*
 * Buffer with float entries
 */
class BufferFloat: public BufferManager<float> {
//public: BufferFloat():BufferManager<float>() {};
};

/*
 * Buffer with unsigned char entries
 */
class BufferChar: public BufferManager<unsigned char> {
//public: BufferChar():BufferManager<unsigned char>() {};
};
