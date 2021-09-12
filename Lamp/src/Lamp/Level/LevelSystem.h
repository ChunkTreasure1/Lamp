#pragma once

#include "Lamp/Core/Core.h"
#include "Level.h"
#include <rapidxml/rapidxml.hpp>

namespace Lamp
{
	class LevelSystem
	{
	public:
		static Ref<Level> LoadLevel(const std::string& path);
		static bool SaveLevel(const std::string& path, Ref<Level>& level);
		static bool SaveLevel(Ref<Level>& level);

		static Ref<Level>& GetCurrentLevel() { return m_CurrentLevel; }
		static LevelEnvironment& GetEnvironment() { return m_CurrentLevel->GetEnvironment(); }
		static void SetCurrentLevel(Ref<Level> level) { m_CurrentLevel = level; }

	private:
		LevelSystem() = delete;
		static std::map<uint32_t, Brush*> LoadBrushes(rapidxml::xml_node<>* pNode);
		static std::map<uint32_t, Entity*> LoadEntities(rapidxml::xml_node<>* pNode);

	private:
		static bool GetValue(char* val, bool& var);
		static bool GetValue(char* val, int& var);
		static bool GetValue(char* val, float& var);
		static bool GetValue(char* val, glm::vec2& var);
		static bool GetValue(char* val, glm::vec3& var);
		static bool GetValue(char* val, glm::vec4& var);

		//CHANGE TO CHAR*
		static std::string ToString(const bool& var);
		static std::string ToString(const int& var);
		static std::string ToString(const float& var);
		static std::string ToString(const glm::vec2& var);
		static std::string ToString(const glm::vec3& var);
		static std::string ToString(const glm::vec4& var);

		static Ref<Level> m_CurrentLevel;

	};
}