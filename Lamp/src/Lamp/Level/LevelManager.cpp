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
		if (m_activeLevel)
		{
			m_activeLevel->Shutdown();
		}
		m_activeLevel = level;
	}

	void LevelManager::Load(const std::filesystem::path& path)
	{
		m_activeLevel = ResourceCache::GetAsset<Level>(path);
	}

	LevelManager* LevelManager::Get()
	{
		return s_instance;
	}

	Ref<Level> LevelManager::GetActive()
	{
		LP_CORE_ASSERT(s_instance, "Instance does not exist!");
		
		return s_instance->m_activeLevel;
	}
}