#include "lppch.h"
#include "IOManager.h"

namespace Lamp
{
	bool IOManager::ReadFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer)
	{
		//Get the file and error check it
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail())
		{
			LP_CORE_ERROR("Could not open " + filePath + "!");
			return false;
		}

		//Seek to end of file and get the file size
		file.seekg(0, std::ios::end);
		int fileSize = (int)file.tellg();
		file.seekg(0, std::ios::beg);

		//Remove file header bytes
		fileSize -= (int)file.tellg();

		//Resize the buffer
		buffer.resize(fileSize);
		file.read((char*) & (buffer[0]), fileSize);
		file.close();

		return true;
	}
}
