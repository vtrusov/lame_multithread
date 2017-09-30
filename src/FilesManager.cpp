
#include "FilesManager.h"

FilesManager::FilesManager(std::string path) : m_path(path) {};

FilesManager::~FilesManager() {};


void FilesManager::collectFiles()
{
#ifdef __GNUC__
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(m_path.c_str())) == NULL) {
		std::cout << "Error opening " << m_path << std::endl;
		return ;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if(dirp->d_type==DT_DIR) continue;
		m_files.push_back(composePath(dirp->d_name));
	}
	closedir(dp);
#else
	std::string searchPath = std::string(m_path + "\\*.*");
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &fd);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
				m_files.push_back(composePath(fd.cFileName));
			}
		}while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
#endif

	eraseNonWav();

	m_fileIterator = m_files.begin();
}

std::string FilesManager::getNextFile()
{
	if(m_fileIterator>=m_files.end()) return std::string("");
	std::string file("");
	file = *m_fileIterator++;
	return file;
}

std::string FilesManager::generateMP3Name(std::string wavFile) {
	std::string mp3File = wavFile;
	return mp3File.replace(wavFile.length()-3, 3, "mp3");
}

void FilesManager::eraseNonWav()
{
	m_files.erase(std::remove_if(m_files.begin(),m_files.end(), [&](std::string& file){
		return !isWavFile(file);
	}), m_files.end());
}

void FilesManager::listFiles()
{
	for(std::string& file: m_files){
		std::cout << file << std::endl;
	}
}

bool FilesManager::isWavFile(std::string file)
{
	if(file.size() < 4) return false;
	if(file.substr(file.size()-4, file.size()) != ".wav") return false;

	try{
		FileHandler *fileHandler = new FileHandler(file);
		bool header = fileHandler->getWavDecoder().checkHeader();
		fileHandler->close();
		delete fileHandler;
		return header;
	} catch (std::exception& exc){
		std::cerr<<exc.what() << std::endl;
	} catch (...) {
		std::cerr<<"Unknown exception occured" << std::endl;
	}

	return false;
}

int FilesManager::getNFiles(){
	return m_files.size();
}


std::string FilesManager::composePath(std::string file)
{
#ifdef __GNUC__
	return std::string(m_path + "/" + file);
#else
	return std::string(m_path + "\\" + file);
#endif
}

