#include "lppch.h"
#include "LevelSystem.h"

#include "Lamp/Objects/Brushes/BrushManager.h"
#include <rapidxml/rapidxml_print.hpp>

#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	Ref<Level> LevelSystem::m_CurrentLevel = nullptr;

	Ref<Level> LevelSystem::LoadLevel(const std::string& path)
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
		ObjectLayerManager::SetCurrentManager(pLevel->GetObjectLayerManager());

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

		if (rapidxml::xml_node<>* pLayers = pRootNode->first_node("Layers"))
		{
			pLevel->GetObjectLayerManager()->SetLayers(LoadLayers(pLayers, pLevel->GetObjectLayerManager()));
		}

		if (rapidxml::xml_node<>* pBrushes = pRootNode->first_node("Brushes"))
		{
			pLevel->GetBrushManager()->SetBrushes(LoadBrushes(pBrushes, pLevel->GetBrushManager()));
		}

		if (rapidxml::xml_node<>* pEntities = pRootNode->first_node("Entities"))
		{
			pLevel->GetEntityManager()->SetEntities(LoadEntities(pEntities, pLevel->GetEntityManager()));
		}

		m_CurrentLevel = pLevel;

		LP_CORE_INFO("Level loaded!");

		EntityManager::SetCurrentManager(pLevel->GetEntityManager());
		BrushManager::SetCurrentManager(pLevel->GetBrushManager());

		return pLevel;
	}

	bool LevelSystem::SaveLevel(const std::string& path, Ref<Level>& level)
	{
		LP_CORE_INFO("Saving level...");

		std::ofstream file;
		rapidxml::xml_document<> doc;
		file.open(path);

		rapidxml::xml_node<>* pRoot = doc.allocate_node(rapidxml::node_element, "Level");
		pRoot->append_attribute(doc.allocate_attribute("name", level->GetName().c_str()));

		/////Brushes/////
		rapidxml::xml_node<>* pBrushes = doc.allocate_node(rapidxml::node_element, "Brushes");
		for (auto& brush : level->GetBrushManager()->GetBrushes())
		{
			rapidxml::xml_node<>* child = doc.allocate_node(rapidxml::node_element, "Brush");

			child->append_attribute(doc.allocate_attribute("lgfPath", brush->GetModel()->GetLGFPath().c_str()));

			char* pPos = doc.allocate_string(ToString(brush->GetPosition()).c_str());
			child->append_attribute(doc.allocate_attribute("position", pPos));

			char* pRot = doc.allocate_string(ToString(brush->GetRotation()).c_str());
			child->append_attribute(doc.allocate_attribute("rotation", pRot));

			char* pScale = doc.allocate_string(ToString(brush->GetScale()).c_str());
			child->append_attribute(doc.allocate_attribute("scale", pScale));

			char* pLayer = doc.allocate_string(ToString((int)brush->GetLayerID()).c_str());
			child->append_attribute(doc.allocate_attribute("layerID", pLayer));

			char* pName = doc.allocate_string(brush->GetName().c_str());
			child->append_attribute(doc.allocate_attribute("name", pName));

			pBrushes->append_node(child);
		}
		pRoot->append_node(pBrushes);

		/////////////////

		/////Layers//////
		rapidxml::xml_node<>* pLayers = doc.allocate_node(rapidxml::node_element, "Layers");
		for (auto& layer : ObjectLayerManager::Get()->GetLayers())
		{
			if (layer.ID == 0)
				continue;

			rapidxml::xml_node<>* child = doc.allocate_node(rapidxml::node_element, "Layer");

			char* pId = doc.allocate_string(ToString((int)layer.ID).c_str());
			child->append_attribute(doc.allocate_attribute("id", pId));

			char* pName = doc.allocate_string(layer.Name.c_str());
			child->append_attribute(doc.allocate_attribute("name", pName));

			char* pDestroyable = doc.allocate_string(ToString(layer.IsDestroyable).c_str());
			child->append_attribute(doc.allocate_attribute("destroyable", pDestroyable));

			pLayers->append_node(child);
		}
		pRoot->append_node(pLayers);
		/////////////////

		////Entities/////
		rapidxml::xml_node<>* pEntities = doc.allocate_node(rapidxml::node_element, "Entities");
		for (auto& entity : level->GetEntityManager()->GetEntities())
		{
			if (!entity->GetSaveable())
			{
				continue;
			}

			rapidxml::xml_node<>* ent = doc.allocate_node(rapidxml::node_element, "Entity");

			char* pName = doc.allocate_string(entity->GetName().c_str());
			ent->append_attribute(doc.allocate_attribute("name", pName));

			char* pPos = doc.allocate_string(ToString(entity->GetPosition()).c_str());
			ent->append_attribute(doc.allocate_attribute("position", pPos));

			char* pRot = doc.allocate_string(ToString(entity->GetRotation()).c_str());
			ent->append_attribute(doc.allocate_attribute("rotation", pRot));

			char* pScale = doc.allocate_string(ToString(entity->GetScale()).c_str());
			ent->append_attribute(doc.allocate_attribute("scale", pScale));

			for (auto component : entity->GetComponents())
			{
				rapidxml::xml_node<>* comp = doc.allocate_node(rapidxml::node_element, "Component");

				std::string str = component->GetName();
				str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

				char* pCompName = doc.allocate_string(str.c_str());
				comp->append_attribute(doc.allocate_attribute("name", pCompName));

				for (auto prop : component->GetComponentProperties().GetProperties())
				{
					rapidxml::xml_node<>* param = doc.allocate_node(rapidxml::node_element, "Param");

					char* pParamName = doc.allocate_string(prop.Name.c_str());
					param->append_attribute(doc.allocate_attribute("name", pParamName));

					char* pType = doc.allocate_string(ComponentProperties::GetStringFromType(prop.PropertyType).c_str());
					param->append_attribute(doc.allocate_attribute("type", pType));

					switch (prop.PropertyType)
					{
					case Lamp::PropertyType::String:
					{
						char* pValue = doc.allocate_string(static_cast<std::string*>(prop.Value)->c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Bool:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<bool*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Int:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<int*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Float:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<float*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Float2:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<glm::vec2*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Float3:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<glm::vec3*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Float4:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<glm::vec4*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Color3:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<glm::vec3*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
						break;
					}
					case Lamp::PropertyType::Color4:
					{
						char* pValue = doc.allocate_string(ToString(*static_cast<glm::vec4*>(prop.Value)).c_str());
						param->append_attribute(doc.allocate_attribute("value", pValue));
					}
					default:
						break;
					}

					comp->append_node(param);
				}

				ent->append_node(comp);
			}

			pEntities->append_node(ent);
		}
		pRoot->append_node(pEntities);
		/////////////////

		/////Level environment/////
		rapidxml::xml_node<>* pLevelEnv = doc.allocate_node(rapidxml::node_element, "LevelEnvironment");
		
		rapidxml::xml_node<>* globalAmbient = doc.allocate_node(rapidxml::node_element, "GlobalAmbient");
		char* pGA = doc.allocate_string(ToString(level->GetEnvironment().GlobalAmbient).c_str());
		globalAmbient->append_attribute(doc.allocate_attribute("value", pGA));

		rapidxml::xml_node<>* cameraValues = doc.allocate_node(rapidxml::node_element, "CameraValues");
		char* pCP = doc.allocate_string(ToString(level->GetEnvironment().CameraPosition).c_str());
		cameraValues->append_attribute(doc.allocate_attribute("position", pCP));

		char* pCR = doc.allocate_string(ToString(level->GetEnvironment().CameraRotation).c_str());
		cameraValues->append_attribute(doc.allocate_attribute("rotation", pCR));

		char* pCF = doc.allocate_string(ToString(level->GetEnvironment().CameraFOV).c_str());
		cameraValues->append_attribute(doc.allocate_attribute("fov", pCF));

		pLevelEnv->append_node(globalAmbient);
		pLevelEnv->append_node(cameraValues);
		pRoot->append_node(pLevelEnv);
		///////////////////////////

		doc.append_node(pRoot);
		file << doc;
		file.close();

		LP_CORE_INFO("Saved level!");

		return true;
	}

	bool LevelSystem::SaveLevel(Ref<Level>& level)
	{
		return SaveLevel(level->GetPath(), level);
	}

	std::vector<ObjectLayer> LevelSystem::LoadLayers(rapidxml::xml_node<>* pNode, Ref<ObjectLayerManager>& objLayerManager)
	{
		std::vector<ObjectLayer> layers;

		for (rapidxml::xml_node<>* pLayer = pNode->first_node("Layer"); pLayer; pLayer = pLayer->next_sibling())
		{
			std::string name(pLayer->first_attribute("name")->value());

			int id;
			GetValue(pLayer->first_attribute("id")->value(), id);

			bool destroyable;
			GetValue(pLayer->first_attribute("destroyable")->value(), destroyable);

			layers.push_back(ObjectLayer((uint32_t)id, name, destroyable));
		}

		layers.push_back(ObjectLayer(0, "Main", false));

		return layers;
	}

	std::vector<Brush*> LevelSystem::LoadBrushes(rapidxml::xml_node<>* pNode, Ref<BrushManager>& brushManager)
	{
		std::vector<Brush*> pBrushes;

		for (rapidxml::xml_node<>* pBrush = pNode->first_node("Brush"); pBrush; pBrush = pBrush->next_sibling())
		{
			std::string path(pBrush->first_attribute("lgfPath")->value());

			glm::vec3 pos(0, 0, 0);
			GetValue(pBrush->first_attribute("position")->value(), pos);

			glm::vec3 rot(0, 0, 0);
			GetValue(pBrush->first_attribute("rotation")->value(), rot);

			glm::vec3 scale(1, 1, 1);
			GetValue(pBrush->first_attribute("scale")->value(), scale);

			int layerID;
			GetValue(pBrush->first_attribute("layerID")->value(), layerID);

			std::string name = pBrush->first_attribute("name")->value();

			pBrushes.push_back(brushManager->Create(path, pos, rot, scale, layerID, name));
		}

		return pBrushes;
	}

	std::vector<Entity*> LevelSystem::LoadEntities(rapidxml::xml_node<>* pNode, Ref<EntityManager>& entityManager)
	{
		std::vector<Entity*> pEntities;

		for (rapidxml::xml_node<>* pEntity = pNode->first_node("Entity"); pEntity; pEntity = pEntity->next_sibling())
		{
			Entity* pEnt = entityManager->Create();

			std::string name = pEntity->first_attribute("name")->value();

			glm::vec3 pos(0, 0, 0);
			GetValue(pEntity->first_attribute("position")->value(), pos);

			glm::vec3 rot(0, 0, 0);
			GetValue(pEntity->first_attribute("rotation")->value(), rot);

			glm::vec3 scale(1, 1, 1);
			GetValue(pEntity->first_attribute("scale")->value(), scale);

			pEnt->SetName(name);
			pEnt->SetPosition(pos);
			pEnt->SetRotation(rot);
			pEnt->SetScale(scale);

			for (rapidxml::xml_node<>* pComponent = pEntity->first_node("Component"); pComponent; pComponent = pComponent->next_sibling())
			{
				auto name = pComponent->first_attribute("name")->value();
				Ref<EntityComponent> pComp = ComponentRegistry::Create(name);
				pComp->MakeOwner(pEnt);
				pComp->Initialize();

				for (rapidxml::xml_node<>* pParam = pComponent->first_node("param"); pParam; pParam = pParam->next_sibling())
				{
					auto paramName = pParam->first_attribute("name")->value();
					auto type = ComponentProperties::GetTypeFromString(pParam->first_attribute("type")->value());

					switch (type)
					{
					case Lamp::PropertyType::String:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								std::string* p = static_cast<std::string*>(prop.Value);
								*p = std::string(pParam->first_attribute("value")->value());
							}
						}

						break;
					}
					case Lamp::PropertyType::Bool:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								bool* p = static_cast<bool*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Int:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								int* p = static_cast<int*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								float* p = static_cast<float*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float2:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								glm::vec2* p = static_cast<glm::vec2*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float3:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float4:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Color3:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					case Lamp::PropertyType::Color4:
					{
						for (auto& prop : pComp->GetComponentProperties().GetProperties())
						{
							if (prop.Name == paramName)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.Value);
								GetValue(pParam->first_attribute("value")->value(), *p);
							}
						}
						break;
					}
					}
				}

				pEnt->AddComponent(pComp);
			}

			pEntities.push_back(pEnt);
		}

		return pEntities;
	}

	//////////Getters//////////
	bool LevelSystem::GetValue(char* val, bool& var)
	{
		if (val)
		{
			var = atoi(val) != 0;
			return true;
		}

		return false;
	}
	bool LevelSystem::GetValue(char* val, int& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
	}
	bool LevelSystem::GetValue(char* val, float& var)
	{
		if (val)
		{
			var = (float)atof(val);
			return true;
		}
		return false;
	}
	bool LevelSystem::GetValue(char* val, glm::vec2& var)
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
	bool LevelSystem::GetValue(char* val, glm::vec3& var)
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
	bool LevelSystem::GetValue(char* val, glm::vec4& var)
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

	std::string LevelSystem::ToString(const bool& var)
	{
		std::string str;
		if (var)
		{
			str = "1";
		}
		else
		{
			str = "0";
		}

		return str;
	}
	std::string LevelSystem::ToString(const int& var)
	{
		return std::to_string(var);
	}
	std::string LevelSystem::ToString(const float& var)
	{
		return std::to_string(var);
	}
	std::string LevelSystem::ToString(const glm::vec2& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y));
		return str;
	}
	std::string LevelSystem::ToString(const glm::vec3& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z));
		return str;
	}
	std::string LevelSystem::ToString(const glm::vec4& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z) + "," + std::to_string(var.w));
		return str;
	}
	//////////////////////////
}