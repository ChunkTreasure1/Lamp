#include "IOManager.h"
#include "Lamp/Errors.h"

#include <fstream>

namespace Lamp
{
	bool IOManager::ReadFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer)
	{
		//Get the file and error check it
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail())
		{
			//perror(filePath.c_str());
			FatalError("Could not open " + filePath + "!");
			return false;
		}

		//Seek to end of file and get the file size
		file.seekg(0, std::ios::end);
		int fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		//Remove file header bytes
		fileSize -= file.tellg();

		//Resize the buffer
		buffer.resize(fileSize);
		file.read((char*) & (buffer[0]), fileSize);
		file.close();

		return true;
	}
}
