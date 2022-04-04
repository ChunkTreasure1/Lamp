#include "lppch.h"
#include "LevelManager.h"

#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	LevelManager* LevelManager::s_instance = nullptr;

	LevelManager::LevelManager()
	{
		if (s_instance)
		{
			LP_CORE_ASSERT(false, "Instance already exists! There should only be one!");
		}

		s_instance = this;
	}

	LevelManager::~LevelManager()
	{
		if (m_activeLevel)
		{
			m_activeLevel->Shutdown();
		}
	}

	void LevelManager::SetActive(Ref<Level> level)
	{
		std::lock_guard lock{ m_mutex };
		m_activeLevel = level;
	}

	Ref<Level> LevelManager::GetActiveLevel()
	{
		return m_activeLevel;
	}

	void LevelManager::Load(const std::filesystem::path& path)
	{
		SetActive(ResourceCache::GetAsset<Level>(path));
	}

	LevelManager* LevelManager::Get()
	{
		return s_instance;
	}

	const bool LevelManager::IsLevelLoaded()
	{
		return s_instance->m_activeLevel != nullptr;
	}

	Ref<Level> LevelManager::GetActive()
	{
		LP_CORE_ASSERT(s_instance, "Instance does not exist!");
		
		return s_instance->GetActiveLevel();
	}
}