#include "lppch.h"
#include "FileSystem.h"

#include "imgui.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Level/LevelSystem.h"
#include "Lamp/Event/ApplicationEvent.h"
#include <ShObjIdl.h>
#include <locale>
#include <codecvt>

namespace Lamp
{
	//Gets the files in a specified folder
	std::vector<std::string> FileSystem::GetFiles(const std::string& path)
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

	//Returns the paths to the files within the assets folder
	std::vector<std::string> FileSystem::GetAssetFolders()
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator("assets"))
		{
			std::string s = entry.path().string();
			if (s.find(".") == std::string::npos)
			{
				folders.push_back(s);
			}
		}

		return folders;
	}

	//Gets the folders in a specified folder
	std::vector<std::string> FileSystem::GetMaterialFolders(const std::string& path)
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
		{
			if (entry.is_directory())
			{
				if (ContainsMaterialFiles(entry.path().string()))
				{
					folders.push_back(entry.path().string());
				}
			}
		}

		return folders;
	}

	std::vector<std::string> FileSystem::GetMaterialFiles(const std::string& path)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			if (s.find(".mtl") != std::string::npos)
			{
				files.push_back(s);
			}
		}

		return files;
	}

	void FileSystem::GetAllMaterialFiles(std::vector<std::string>& folders, std::vector<std::string>& files)
	{
		if (folders.size() == 0)
		{
			return;
		}

		for (int i = 0; i < folders.size(); i++)
		{
			std::vector<std::string> f = Lamp::FileSystem::GetMaterialFiles(folders[i]);

			for (int j = 0; j < f.size(); j++)
			{
				files.push_back(f[j]);
			}
			GetAllMaterialFiles(GetFolders(folders[i]), files);
		}
	}

	//Gets the folders in a specified folder
	std::vector<std::string> FileSystem::GetFolders(const std::string& path)
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
	bool FileSystem::ContainsMaterialFiles(const std::string& path)
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.path().extension() == ".mtl")
			{
				return true;
			}
		}

		return false;
	}

	std::vector<std::string> FileSystem::GetBrushFiles(const std::string& path)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string s = entry.path().string();
			if (s.find(".lgf") != std::string::npos)
			{
				files.push_back(s);
			}
		}

		return files;
	}

	void FileSystem::PrintBrushes(std::vector<std::string>& folders, int startID)
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

			std::vector<std::string> files = Lamp::FileSystem::GetBrushFiles(folders[i]);

			for (int j = 0; j < files.size(); j++)
			{
				startID++;
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				std::string p = files[j];
				if (p.find(".spec") != std::string::npos)
				{
					continue;
				}

				std::size_t posp = p.find_last_of("/\\");
				p = p.substr(posp + 1);

				std::size_t lgfPos = p.find_last_of(".");
				p = p.substr(0, lgfPos);

				ImGui::TreeNodeEx((void*)(intptr_t)startID, nodeFlags, p.c_str());
				if (ImGui::BeginDragDropSource())
				{
					const char* path = files[j].c_str();
					ImGui::SetDragDropPayload("BRUSH_ITEM", path, sizeof(char) * (files[j].length() + 1), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}
				if (ImGui::IsItemClicked())
				{
					File f(files[j]);
					AppItemClickedEvent e(f);
					Application::Get().OnEvent(e);
				}
			}
			PrintBrushes(GetFolders(folders[i]), startID);
		}
	}
}