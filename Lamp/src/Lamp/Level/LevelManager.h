#pragma once

#include "Level.h"

namespace Lamp
{
	class LevelManager
	{
	public:
		LevelManager();
		~LevelManager();

		void SetActive(Ref<Level> level);
		void Load(const std::filesystem::path& path);

		static LevelManager* Get();
		static Ref<Level> GetActive();

	private:

		static LevelManager* s_instance;
		Ref<Level> m_activeLevel;
	};
}