#include "lppch.h"
#include "FileSystem.h"

#include "imgui.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Level/LevelSystem.h"

namespace Lamp
{
	//Returns the paths to the files within the assets folder
	std::vector<std::string> FileSystem::GetAssetFolders()
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator("engine"))
		{
			std::string s = entry.path().string();
			if (s.find(".") == std::string::npos)
			{
				folders.push_back(s);
			}
		}

		return folders;
	}

	//Gets the files in a specified folder
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

	std::vector<std::string> FileSystem::GetLevelFiles(std::string& path)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			if (s.find(".level") != std::string::npos)
			{
				files.push_back(s);
			}
		}

		return files;
	}

	//Gets the folders in a specified folder
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

	//Returns whether or not a folder contains folders
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
	void FileSystem::PrintFoldersAndFiles(std::vector<std::string>& folders, int startId)
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

				for (int j = 0; j < files.size(); j++)
				{
					startId++;
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

					std::string p = files[j];
					std::size_t posp = p.find_last_of("/\\");
					p = p.substr(posp + 1);

					ImGui::TreeNodeEx((void*)(intptr_t)startId, nodeFlags, p.c_str());
					if (ImGui::IsItemClicked())
					{
						Application::Get().OnItemClicked(File(files[j]));
					}
				}

				PrintFoldersAndFiles(Lamp::FileSystem::GetFolders(folders[i]), startId);
				ImGui::TreePop();
			}
		}
	}

	//Prints all the availible folders and files (CALL ONLY WHEN IMGUI CONTEXT EXISTS OR IT WILL CRASH!)
	void FileSystem::PrintLevelFiles(std::vector<std::string>& folders, int startID)
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

			std::vector<std::string> files = Lamp::FileSystem::GetLevelFiles(folders[i]);

			for (int j = 0; j < files.size(); j++)
			{
				//startId++;
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				std::string p = files[j];
				std::size_t posp = p.find_last_of("/\\");
				p = p.substr(posp + 1);

				if (p.find(".level"))
				{
					if (ImGui::MenuItem(p.c_str()))
					{
						File f = File(p.c_str());

						if (f.GetFileType() == FileType::Level)
						{
							Lamp::LevelSystem::SaveLevel("engine/levels/" + Lamp::LevelSystem::GetCurrentLevel()->GetName() + ".level", Lamp::LevelSystem::GetCurrentLevel());
							LevelSystem::LoadLevel("engine/levels/" + f.GetPath());
						}
					}
				}
			}

			PrintLevelFiles(GetFolders(folders[i]), startID);
		}
	}
}