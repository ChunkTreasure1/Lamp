#include "lppch.h"
#include "FileSystem.h"

namespace Lamp
{
	std::vector<std::string> FileSystem::GetAssetFolders()
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator("Assets"))
		{
			std::string s = entry.path().string();
			std::size_t pos = s.find_last_of("/\\");

			s = s.substr(pos + 1);
			if (s.find(".") == std::string::npos)
			{
				folders.push_back(s);
			}
		}

		return folders;
	}

	std::vector<std::string> FileSystem::GetFiles(std::string & path)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			std::size_t pos = s.find_first_of("/\\");

			s = s.substr(pos + 1);

			if (s.find(".") != std::string::npos)
			{
				files.push_back(s);
			}
		}

		return files;
	}

	std::vector<std::string> FileSystem::GetFolders(std::string & path)
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			std::size_t pos = s.find_last_of("/\\");

			s = s.substr(pos + 1);
			if (s.find(".") == std::string::npos)
			{
				folders.push_back(s);
			}
		}

		return folders;
	}

	bool FileSystem::ContainsFolder(std::string & path)
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			std::size_t pos = s.find_last_of("/\\");

			s = s.substr(pos + 1);
			if (s.find(".") == std::string::npos)
			{
				return true;
			}
		}

		return false;
	}
}