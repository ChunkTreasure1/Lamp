#pragma once
#include <vector>
#include <string>

namespace Lamp
{
	class IOManager
	{
	public:

		static bool ReadFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer);
	};
}

