#include "lppch.h"
#include "Sandbox.h"

#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include "Lamp/Level/LevelManager.h"

namespace Sandbox
{
	void Sandbox::SaveLevelAs()
	{
		std::filesystem::path filepath = Lamp::FileDialogs::SaveFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			Lamp::LevelManager::GetActive()->Path = filepath;
			g_pEnv->pAssetManager->SaveAsset(Lamp::LevelManager::GetActive());
		}
	}

	void Sandbox::OpenLevel()
	{
		std::filesystem::path filepath = Lamp::FileDialogs::OpenFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			OpenLevel(filepath);
		}
	}

	void Sandbox::OpenLevel(const std::filesystem::path& path)
	{
		m_pLevel = Lamp::ResourceCache::GetAsset<Lamp::Level>(path);
		m_pSelectedObject = nullptr;
	}

	void Sandbox::NewLevel()
	{
		if (Lamp::LevelManager::GetActive() && !Lamp::LevelManager::GetActive()->Path.empty())
		{
			g_pEnv->pAssetManager->SaveAsset(Lamp::LevelManager::GetActive());
		}

		m_pLevel = CreateRef<Lamp::Level>("New Level");
		Lamp::LevelManager::Get()->SetActive(m_pLevel);
		m_pSelectedObject = nullptr;
	}

	void Sandbox::Undo()
	{
		m_ActionHandler.Undo();
	}

	void Sandbox::Redo()
	{
		m_ActionHandler.Redo();
	}
}