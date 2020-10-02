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

		static Ref<Level>& GetCurrentLevel() { return m_CurrentLevel; }
		static LevelEnvironment& GetEnvironment() { return m_CurrentLevel->GetEnvironment(); }

	private:
		LevelSystem() = delete;
		static std::vector<Brush*> LoadBrushes(rapidxml::xml_node<>* pNode, Ref<BrushManager>& brushManager);
		static std::vector<Entity*> LoadEntities(rapidxml::xml_node<>* pNode, Ref<EntityManager>& entityManager);
		static std::vector<ObjectLayer> LoadLayers(rapidxml::xml_node<>* pNode, Ref<ObjectLayerManager>& objLayerManager);

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