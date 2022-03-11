#include "lppch.h"
#include "SandboxLayer.h"

#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include "Lamp/Level/LevelManager.h"

namespace Sandbox
{
	void SandboxLayer::SaveLevelAs()
	{
		std::filesystem::path filepath = Lamp::FileDialogs::SaveFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			Lamp::LevelManager::GetActive()->Path = filepath;
			g_pEnv->pAssetManager->SaveAsset(Lamp::LevelManager::GetActive());
		}
	}

	void SandboxLayer::OpenLevel()
	{
		std::filesystem::path filepath = Lamp::FileDialogs::OpenFile("Lamp Level (*.level)\0*.level\0");
		if (!filepath.empty())
		{
			OpenLevel(filepath);
		}
	}

	void SandboxLayer::OpenLevel(const std::filesystem::path& path)
	{
		Lamp::LevelManager::Get()->Load(path);
		m_pSelectedObject = nullptr;
	}

	void SandboxLayer::NewLevel()
	{
		if (Lamp::LevelManager::IsLevelLoaded() && !Lamp::LevelManager::GetActive()->Path.empty())
		{
			g_pEnv->pAssetManager->SaveAsset(Lamp::LevelManager::GetActive());
		}

		Lamp::LevelManager::Get()->SetActive(CreateRef<Lamp::Level>("New Level"));
		m_pSelectedObject = nullptr;
	}

	void SandboxLayer::Undo()
	{
		//Get active window
		CommandStack* cmdStack = nullptr;
		
		if (m_perspectiveOpen)
		{
			cmdStack = &m_perspectiveCommands;
		}
		else
		{
			for (auto window : m_windows)
			{
				if (window->IsFocused())
				{
					cmdStack = &window->GetCommandStack();
					break;
				}
			}
		}

		cmdStack->Undo();
	}

	void SandboxLayer::Redo()
	{
	}
}