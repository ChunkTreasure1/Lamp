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

	std::vector<std::string> FileSystem::GetLevelFiles(const std::string& path)
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

	//Returns whether or not a folder contains folders
	bool FileSystem::ContainsFolder(const std::string& path)
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

	//Prints all the available folders and files (CALL ONLY WHEN IMGUI CONTEXT EXISTS OR IT WILL CRASH!)
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
						File f(files[j]);
						AppItemClickedEvent e(f);
						Application::Get().OnEvent(e);
					}
				}

				PrintFoldersAndFiles(Lamp::FileSystem::GetFolders(folders[i]), startId);
				ImGui::TreePop();
			}
		}
	}

	//Prints all the available folders and files (CALL ONLY WHEN IMGUI CONTEXT EXISTS OR IT WILL CRASH!)
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
				startID++;
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
					ImGui::SetDragDropPayload("BRUSH_ITEM", path, sizeof(char) * files[j].length(), ImGuiCond_Once);
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