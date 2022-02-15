#pragma once

#include "Lamp/Core/Buffer.h"

namespace Lamp
{
	class FileSystem
	{
	public:
		static std::vector<std::string> GetAssetFolders();
		static std::vector<std::string> GetEngineFolders();
		static std::vector<std::string> GetFiles(const std::string& path);
		static std::vector<std::filesystem::path> GetMaterialFolders(const std::filesystem::path& path);
		static std::vector<std::string> GeFilesOfType(const std::string& path, const std::string& type);
		static std::vector<std::string> GetFolders(const std::string& path);
		static std::vector<std::string> GetBrushFiles(const std::string& path);

		static bool WriteBytes(const std::filesystem::path& path, const Buffer& buffer);
		static Buffer ReadBytes(const std::filesystem::path& path);

		static void GetAllFilesOfType(const std::string& type, const std::string& path, std::vector<std::string>& outFiles);
		static bool ContainsMaterialFiles(const std::string& path);
		static void PrintBrushes(std::vector<std::string>& files, int startID = -1);
	};
}