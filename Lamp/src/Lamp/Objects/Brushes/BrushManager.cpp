#include "lppch.h"
#include "BrushManager.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	BrushManager::BrushManager()
	{
	}

	BrushManager::~BrushManager()
	{
		m_Brushes.clear();
	}

	Brush2D* BrushManager::Create2D(const std::string& path)
	{
		auto brush = new Brush2D(path);
		brush->SetLayerID(0);

		m_2DBrushes.push_back(brush);

		ObjectLayerManager::Get()->AddToLayer(brush, 0);
		return brush;
	}

	Brush2D* BrushManager::Create2D(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, uint32_t layerId, const std::string& name)
	{
		auto brush = new Brush2D(path);

		brush->SetPosition(pos);
		brush->SetRotation(rot);
		brush->SetScale(scale);
		brush->SetLayerID(layerId);
		brush->SetName(name);

		m_2DBrushes.push_back(brush);

		ObjectLayerManager::Get()->AddToLayer(brush, layerId);
		return brush;
	}

	Brush* BrushManager::Create(const std::string& path)
	{
		Ref<Model> model = ResourceCache::GetModel(path);

		auto brush = new Brush(model);
		brush->SetLayerID(0);

		m_Brushes.emplace(std::make_pair(brush->GetID(), brush));

		ObjectLayerManager::Get()->AddToLayer(brush, 0);
		return brush;
	}

	Brush* BrushManager::Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, uint32_t layerId, const std::string& name)
	{
		Ref<Model> model = ResourceCache::GetModel(path);
		auto brush = new Brush(model);

		brush->SetPosition(pos);
		brush->SetRotation(rot);
		brush->SetScale(scale);
		brush->SetLayerID(layerId);
		brush->SetName(name);

		m_Brushes.emplace(std::make_pair(brush->GetID(), brush));

		ObjectLayerManager::Get()->AddToLayer(brush, layerId);
		return brush;
	}

	Brush* BrushManager::Create(Brush* main)
	{
		Ref<Model> model = ResourceCache::GetModel(main->GetModel()->GetLGFPath());

		auto brush = new Brush(model);
		brush->SetLayerID(main->GetLayerID());

		m_Brushes.emplace(std::make_pair(brush->GetID(), brush));
		ObjectLayerManager::Get()->AddToLayer(brush, brush->GetLayerID());

		return brush;
	}

	void BrushManager::Remove(Brush* brush)
	{
		if (m_Brushes.size() == 0 && !brush)
		{
			return;
		}

		m_Brushes.erase(brush->GetID());
	}

	void BrushManager::Remove(Brush2D* brush)
	{
		auto it = std::find(m_2DBrushes.begin(), m_2DBrushes.end(), brush);
		if (it != m_2DBrushes.end())
		{
			m_2DBrushes.erase(it);
		}
	}
}