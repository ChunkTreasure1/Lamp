#pragma once

#include <filesystem>

namespace Sandbox3D
{
	class AssetManagerPanel
	{
	public:
		AssetManagerPanel();

		void OnImGuiRender();

	private:
		bool m_isOpen = true;
		std::filesystem::path m_currentDirectory;
	};
}