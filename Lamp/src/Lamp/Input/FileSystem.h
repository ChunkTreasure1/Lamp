#pragma once

namespace Lamp
{
	enum FileType
	{
		FileType_Empty = 0,
		FileType_PNG,
		FileType_TXT
	};

	class File
	{
	public:
		File(const std::string& path)
			: m_Path(path)
		{
			std::string s = path;
			std::size_t pos = s.find_last_of(".");

			s = s.substr(pos + 1);

			if (s == "PNG" || s == "png")
			{
				m_FileType = FileType_PNG;
			}
			else if (s == "TXT" || s == "txt")
			{
				m_FileType = FileType_TXT;
			}
			else
			{
				m_FileType = FileType_Empty;
			}
		}

		inline const std::string GetPath() const { return m_Path; }
		inline const FileType GetFileType() const { return m_FileType; }

	private:
		FileType m_FileType;
		std::string m_Path;
	};

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