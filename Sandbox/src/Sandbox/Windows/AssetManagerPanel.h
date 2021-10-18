#pragma once

#include <filesystem>
#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Sandbox
{
	class AssetManagerPanel
	{
	public:
		AssetManagerPanel();

		void OnImGuiRender();

		bool& GetIsOpen() { return m_isOpen; }

	private:
		bool m_isOpen = true;
		std::filesystem::path m_currentDirectory;
		Ref<Lamp::Texture2D> m_directoryTexture;
		Ref<Lamp::Texture2D> m_fileTexture;
		std::unordered_map<Lamp::AssetType, Ref<Lamp::Texture2D>> m_Icons;
	};
}