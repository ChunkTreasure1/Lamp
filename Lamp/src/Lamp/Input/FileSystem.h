#pragma once

namespace Lamp
{
	class FileSystem
	{
	public:
		static std::vector<std::string> GetAssetFolders();
		static std::vector<std::string> GetFiles(std::string& path);
		static std::vector<std::string> GetFolders(std::string& path);

		static bool ContainsFolder(std::string& path);
		static void PrintFoldersAndFiles(std::vector<std::string>& files, int startId = -1);

	};
}