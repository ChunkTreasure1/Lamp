#pragma once

namespace Lamp
{
	enum class FileType
	{
		Empty = 0,
		Texture,
		Text,
		Level,
		Brush
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
				m_FileType = FileType::Texture;
			}
			else if (s == "TXT" || s == "txt")
			{
				m_FileType = FileType::Text;
			}
			else if (s == "LEVEL" || s == "level")
			{
				m_FileType = FileType::Level;
			}
			else if (s == "lgf" || s == "LGF")
			{
				m_FileType = FileType::Brush;
			}
			else
			{
				m_FileType = FileType::Empty;
			}
		}

		inline const std::string& GetPath() const { return m_Path; }
		inline const FileType GetFileType() const { return m_FileType; }

	private:
		FileType m_FileType;
		std::string m_Path;
	};

	class FileSystem
	{
	public:
		static std::vector<std::string> GetAssetFolders();
		static std::vector<std::string> GetFiles(const std::string& path);
		static std::vector<std::string> GetMaterialFolders(const std::string& path);
		static std::vector<std::string> GetMaterialFiles(const std::string& path);
		static std::vector<std::string> GetFolders(const std::string& path);
		static std::vector<std::string> GetBrushFiles(const std::string& path);

		static void GetAllMaterialFiles(std::vector<std::string>& folders, std::vector<std::string>& files);
		static bool ContainsMaterialFiles(const std::string& path);
		static void PrintBrushes(std::vector<std::string>& files, int startID = -1);
	};
}