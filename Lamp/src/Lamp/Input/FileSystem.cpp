#include "lppch.h"
#include "FileSystem.h"

#include "imgui.h"

namespace Lamp
{
	std::vector<std::string> FileSystem::GetAssetFolders()
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator("Assets"))
		{
			std::string s = entry.path().string();
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
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			std::string s = entry.path().string();
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

	//Prints all the availible folders and files (CALL ONLY WHEN IMGUI CONTEXT EXISTS OR IT WILL CRASH!)
	void FileSystem::PrintFoldersAndFiles(std::vector<std::string>& folders)
	{
		if (folders.size() == 0)
		{
			return;
		}
		for (int i = 0; i < folders.size(); i++)
		{
			std::string s = folders[i];
			std::size_t pos = s.find_last_of("/\\");
			s = s.substr(pos + 1);

			if (ImGui::TreeNode(s.c_str()))
			{
				std::string s = folders[i].c_str();
				std::vector<std::string> files = Lamp::FileSystem::GetFiles(folders[i]);

				PrintFoldersAndFiles(Lamp::FileSystem::GetFolders(folders[i]));

				for (int i = 0; i < files.size(); i++)
				{
					std::string p = files[i];
					std::size_t posp = p.find_last_of("/\\");
					p = p.substr(posp + 1);

					ImGui::Text(p.c_str());
				}

				ImGui::TreePop();
			}
		}
	}
}