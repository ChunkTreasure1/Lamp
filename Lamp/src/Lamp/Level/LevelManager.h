#pragma once

#include "Level.h"

#include <mutex>

namespace Lamp
{
	class LevelManager
	{
	public:
		LevelManager();
		~LevelManager();

		void SetActive(Ref<Level> level);
		Ref<Level> GetActiveLevel();
		void Load(const std::filesystem::path& path);

		static LevelManager* Get();
		static const bool IsLevelLoaded();
		static Ref<Level> GetActive();

	private:

		static LevelManager* s_instance;
		
		std::mutex m_mutex;
		Ref<Level> m_activeLevel;
	};
}