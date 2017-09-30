I. Project description.
-----------------------
The project is designed to convert WAV files within the specified directory to MP3 format.
The application is intended to convert uncompressed PCM data (WAVE_FORMAT_PCM) as well 
as IEEE float compressed PCM format (WAVE_FORMAT_IEEE_FLOAT).
The following files are included in the project:
include:              	folder with headers
   BufferManager.h		BufferManager header
   ConverterConcurrent.h	ConverterConcurrent header
   Converter.h			Converter and ConverterInterface header
   DataManager.h		DataManager header
   FileHandler.h		FileHandler header
   FilesManager.h		FilesManager header
   LameEncoder.h		LameEncoder header
   MP3Manager.h			MP3Manager header
   Mutex.h			Mutex and Cond header
   MutexManager.h		MutexManager header
   QueueManager.h		QueueManager header
   ThreadFactory.h		ThreadFactory header
   Thread.h			Thread header
   ThreadManager.h		ThreadManager header
   ThreadWorker.h		ThreadWorker header
   WavDecoder.h			WavDecoder header
src:			folder with source files
   BufferManager.cpp
   ConverterConcurrent.cpp
   Converter.cpp
   DataManager.cpp
   FileHandler.cpp		FileHandle implementation
   FilesManager.cpp		FilesManager implementation
   LameEncoder.cpp		LameEncoder implementation
   MP3Manager.cpp		MP3Manager implementation
   Mutex.cpp			Mutex and Cond implementation
   MutexManager.cpp		MutexManager implementation
   QueueManager.cpp		QueueManager implementation
   Thread.cpp			Thread implementation
   ThreadFactory.cpp		ThreadFactory implementation
   ThreadManager.cpp		ThreadManager implementation
   ThreadWorker.cpp		ThreadWorker implementation
   WavConverter.cpp		WavConverter application
   WavDecoder.cpp		WavDecoder header
lame:			folder with LAME header
   lame.h			lame header
pthread:		folder with pthread headers
   pthread.h			pthread header
   sched.h
   semaphore.h
validation:		folder with validation files
   sine_wave_concurrent.png	the waveform of 200hz sine wave before and after conversion (concurrent mode)
   sine_wave_concurrent_PrebufferDisabled.png		
			the waveform of 200hz sine wave after concurrent conversion with Prebuffer enabled and disabled
libmp3lame.dll			lame windows dll library (ver. 3.99.5)
libmp3lame.lib			lame windows lib library (ver. 3.99.5)
pthreadVC2.dll			pthread windows dll library (pthreads-w32 2.9.1)
pthreadVC2.lib			pthread windows dll library (pthreads-w32 2.9.1)
Makefile			Makefile for GNU make (Linux)
Makefile.win			Makefile for VS nmake (Windows)

Program can be built on linux with 

	$ make

command, which produces WavConverter binary. The linux build requires 
lame libraries installed on the system, and it statically links lame and 
pthread libraries to the binary. To run the application:
	
	$ ./WavConverter /path/to/directory

where /path/to/directory is the path to the directory with WAV files.
On windows, program can be built with

	> nmake -f Makefile.win

and produces WavConverter.exe executable. To run the executable, libmp3lame.dll 
and pthreadVC2.dll libraries should be placed in the same folder. To run the application:
	
	> WavConverter.exe F:\path\to\directory

where F:\path\to\directory is the path to the directory with WAV files.
The program creates converted MP3 files in the same directory.


II. Implementation.
-----------------------
The source code implemented using C++11 standard.
Templated classes are implemented in header files (which are placed in include/ directory), 
while non-templated classes consist of interface placed in a header file with the implementation 
in source files (placed in src/ folder).


III. Design
-----------------------
The application uses LAME encoder library to convert PCM encoded WAV files
to MP3 format. Processing is done using all available CPU logical cores. Multithreading is 
implemented by means of using Posix Threads.

1. MP3 encoder
The input WAV files are handled by FileHandler class instances. WAV headers are decoded
by WavDecoder. LameEncoder (a wrapper class for the LAME library) uses parameters of decoded
WAV header to initialize LAME encoder. Also, LameEncoder performs conversion of PCM data 
to MP3 format. Converter class operates with FileHandler and LameEncoder instances to read 
and convert input data. Encoded data is stored into MP3 files by MP3Manager class instance.

1.1. LAME encoder
LAME encoder natively does not support threaded encoding, meaning that WAV files processed 
linearly (each read data block should be encoded by the same LAME instance which encoded 
previous data block from the same PCM file). Its due to LAME internal buffers which keep 
encoding information from previous steps to glue MP3 frames.

2. Multithreading
Each Posix Thread instance is wrapped by ThreadWorker class, and run until no data to process 
is left. ThreadWorker class manipulate with DataManager and Converter instances to read, 
process and write data within a single Posix Thread. Threads are created by ThreadFactory
instance and being managed by ThreadManager class instance. DataManager is used to distribute 
data to threads. 
Two multithreading modes are implemented: parallel and concurrent.

2.1. Parallel data processing
In parallel mode, each thread converts given WAV file from the beginning to the end. This
means, that each WAV file will be processed by the same LAME encoder instance. Processing 
steps include reading of PCM data, converting it with LAME encoder and writing of converted
data to MP3 file. This approach implies a standard PCM to MP3 encoding routine, but executed
on multiple threads in parallel. The disadvantage of this approach shows up when number of
WAV files to process are less than a number of available threads, meaning that some threads may
stand idle.

2.2. Concurrent data processing
Concurrent processing mode implies that multiple threads are processing the same data file.
Reading of PCM data is performed in random order -- once one thread finishes reading of file
it unlocks mutex variable to give access to the file for the next waiting thread. The PCM 
reading order is stored in access queue and used at latter writing stage to restore proper 
writing ordering, meaning that each thread has to wait until all previous threads finish to
write converted data.
In concurrent mode, each thread has its own instance of LAME encoder to convert PCM data.
It means that each LAME instance will perform non-linear data conversion, since PCM blocks
are distributed among threads randomly. The main problem of this approach is that MP3 frames
cannot be glued properly in output MP3 file. To emulate linear PCM processing in each LAME
instance, PCM conversion is performed in two steps: processing of previous PCM buffer and 
processing of current PCM buffer. Previous PCM buffer (so-called "PreBuffer") is a PCM data
read by the previous thread, and current PCM buffer is the data read by the current thread. In 
a concurrent mode each thread stores data it got in common data storage, so the next thread
can use it as PreBuffer data. Treating LAME encoder with PreBuffer data imitates the behaviour
of linear data processing, so MP3 frames get to glue correctly. The disadvantage of this 
method is that it increases data volume processed by each thread, but benefits in case of
few files to process.


3. Execution time
-----------------------
Measurement of the execution time was performed on 8-core CPU (Intel(R) Core(TM)i7@2.70GHz),
with data stored on the internal SSD drive. The following WAV files were selected for the testing 
purposes:
	* PCM encoded WAV file, size 9357 KB;
	* IEEE float encoded WAV file, size 81565 KB.

The following table shows execution time on a single core:
-----------------------------------------------------------------
single file	|     9357 KB (PCM)	|    81565 KB (float)	|
-----------------------------------------------------------------
concurrent	|	2.318s		|	9.962s		|
parallel	|	1,667s		|	6.983s		|
-----------------------------------------------------------------
As expected, application in concurrent mode takes more time to process a single file on a 
single core.

To check performance in multithreaded mode few filesets were prepared. To check the correlation
between execution time and a number of files, multiple copies of the test files were created.
-----------------------------------------------------------------
single file	|     9357 KB (PCM)	|    81565 KB (float)	|
-----------------------------------------------------------------
concurrent	|	0.499s		|	2.241s		|	
parallel	|	1,649s		|	6.981s		|
-----------------------------------------------------------------
4 files		|   4 x 9357 KB (PCM)	|  4 x 81565 KB (float)	|
-----------------------------------------------------------------
concurrent	|	2.017s		|	9.146s		|
parallel	|	2.135s		|	8.834s		|
-----------------------------------------------------------------
8 files		|   8 x 9357 KB (PCM)	|  8 x 81565 KB (float)	|
-----------------------------------------------------------------
concurrent	|	4.159s		|	18.981s		|
parallel	|	2.856s		|	12.194s		|
-----------------------------------------------------------------
23 files	|  23 x 9357 KB (PCM)	| 23 x 81565 KB (float)	|
-----------------------------------------------------------------
concurrent	|	12.788s		|	62.837s		|
parallel	|	8.676s		|	55.861s		|
-----------------------------------------------------------------
In case of a single file, the execution time of the concurrent mode is significantly lower,
but with increasing number of files, the parallel mode finishes processing faster.

To take into account this specifics of the application, execution mode is selected based on the 
number of input files. This approach allows to effectively use all available CPU resources to convert
WAV files in the given folder.

4. Validation
-----------------------
Validation of the converted MP3 files was done using sample sin wave (200Hz). It was checked for 
distortion of the waveform after conversion. The results are stored in validation/ folder.
"sine_wave_concurrent_PrebufferDisabled.png" file shows the impact of the Prebuffer on the converted mp3 data.
With disabled prebuffer waveform suffers from distortions, meaning that converted audio files have
inaudible quality. With enabled prebuffer waveform is smooth, meaning that conversion was successful.
"sine_wave_concurrent.png" shows the comparison of original WAV file to converted (in the concurrent mode with
enabled prebuffer). It can be seen that both waveforms have equal shape.