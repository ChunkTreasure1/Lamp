#include "lppch.h"
#include "LevelLoader.h"
#include <rapidxml/rapidxml.hpp>

namespace Lamp
{
	static bool GetValue(char* val, bool& var)
	{
		if (val)
		{
			var = atoi(val) != 0;
			return true;
		}

		return false;
	}
	static bool GetValue(char* val, int& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
	}
	static bool GetValue(char* val, float& var)
	{
		if (val)
		{
			var = (float)atof(val);
			return true;
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec2& var)
	{
		if (val)
		{
			float x, y;
			if (sscanf(val, "%f,%f", &x, &y) == 2)
			{
				var = glm::vec2(x, y);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec3& var)
	{
		if (val)
		{
			float x, y, z;
			if (sscanf(val, "%f,%f,%f", &x, &y, &z) == 3)
			{
				var = glm::vec3(x, y, z);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec4& var)
	{
		if (val)
		{
			float x, y, z, w;
			if (sscanf(val, "%f,%f,%f,%f", &x, &y, &z, &w) == 4)
			{
				var = glm::vec4(x, y, z, w);
				return true;
			}
		}
		return false;
	}

	void LevelLoader::LoadLevel(LevelLoadData& data, const std::string& path)
	{
		LP_CORE_INFO("Loading level...");

		Ref<Level> pLevel;

		rapidxml::xml_document<> file;
		rapidxml::xml_node<>* pRootNode;

		std::ifstream levelFile(path);
		std::vector<char> buffer((std::istreambuf_iterator<char>(levelFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');

		file.parse<0>(&buffer[0]);
		pRootNode = file.first_node("Level");

		pLevel = CreateRef<Level>(pRootNode->first_attribute("name")->value(), path);

		if (rapidxml::xml_node<>* pLevelEnv = pRootNode->first_node("LevelEnvironment"))
		{
			if (auto pGA = pLevelEnv->first_node("GlobalAmbient"))
			{
				glm::vec3 gA;
				GetValue(pGA->first_attribute("value")->value(), gA);
				pLevel->GetEnvironment().GlobalAmbient = gA;
			}

			if (auto pCP = pLevelEnv->first_node("CameraValues"))
			{
				glm::vec3 cP;
				GetValue(pCP->first_attribute("position")->value(), cP);

				glm::vec3 cR;
				GetValue(pCP->first_attribute("rotation")->value(), cR);

				float fov;
				GetValue(pCP->first_attribute("fov")->value(), fov);

				pLevel->GetEnvironment().CameraPosition = cP;
				pLevel->GetEnvironment().CameraRotation = cR;
				pLevel->GetEnvironment().CameraFOV = fov;
			}
		}

		//if (rapidxml::xml_node<>* pLayers = pRootNode->first_node("Layers"))
		//{
		//	ObjectLayerManager::Get()->SetLayers(LoadLayers(pLayers, ObjectLayerManager::Get()));
		//}

		//if (rapidxml::xml_node<>* pBrushes = pRootNode->first_node("Brushes"))
		//{
		//	BrushManager::Get()->SetBrushes(LoadBrushes(pBrushes, BrushManager::Get()));
		//}

		//if (rapidxml::xml_node<>* pEntities = pRootNode->first_node("Entities"))
		//{
		//	EntityManager::Get()->SetEntities(LoadEntities(pEntities, EntityManager::Get()));
		//}

		//m_CurrentLevel = pLevel;

		LP_CORE_INFO("Level loaded!");

		//return pLevel;
	}
}