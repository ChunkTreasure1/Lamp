#pragma once

#include <filesystem>
#include "Lamp/Rendering/Texture2D/Texture2D.h"

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
		Ref<Lamp::Texture2D> m_directoryTexture;
		Ref<Lamp::Texture2D> m_fileTexture;
	};
}