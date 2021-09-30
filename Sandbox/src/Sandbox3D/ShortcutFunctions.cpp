#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Sandbox3D
{
	void Sandbox3D::SaveLevelAs()
	{
		std::string filepath = Lamp::FileDialogs::SaveFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			g_pEnv->pLevel->Path = filepath;
			g_pEnv->pAssetManager->SaveAsset(g_pEnv->pLevel);
		}
	}

	void Sandbox3D::OpenLevel()
	{
		std::string filepath = Lamp::FileDialogs::OpenFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			OpenLevel(filepath);
		}
	}

	void Sandbox3D::OpenLevel(const std::filesystem::path& path)
	{
		m_pLevel = Lamp::ResourceCache::GetAsset<Lamp::Level>(path);
		m_pSelectedObject = nullptr;
	}

	void Sandbox3D::NewLevel()
	{
		if (!g_pEnv->pLevel->Path.empty())
		{
			g_pEnv->pAssetManager->SaveAsset(g_pEnv->pLevel);
		}

		m_pLevel = CreateRef<Lamp::Level>("New Level");
		g_pEnv->pLevel = m_pLevel;
		m_pSelectedObject = nullptr;
	}

	void Sandbox3D::Undo()
	{
		m_ActionHandler.Undo();
	}

	void Sandbox3D::Redo()
	{
		m_ActionHandler.Redo();
	}
}