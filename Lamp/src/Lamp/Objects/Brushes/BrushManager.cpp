#include "lppch.h"
#include "BrushManager.h"

#include "Lamp/Meshes/GeometrySystem.h"
#include "Lamp/Physics/Ray.h"
#include "Lamp/Objects/Entity/Base/Physical/PhysicalEntity.h"
#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	Ref<BrushManager> BrushManager::s_Manager = nullptr;

	BrushManager::BrushManager()
	{
	}

	BrushManager::~BrushManager()
	{
		for (auto& brush : m_Brushes)
		{
			delete brush;
		}

		m_Brushes.clear();
	}

	Brush* BrushManager::Create(const std::string& path)
	{
		auto brush = new Brush(GeometrySystem::LoadFromFile(path));
		m_Brushes.push_back(brush);

		ObjectLayerManager::Get()->AddToLayer(brush, 0);
		PhysicsEngine::Get()->AddEntity(brush->GetPhysicalEntity());
		return brush;
	}

	Brush* BrushManager::Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, uint32_t layerId, const std::string& name)
	{
		auto brush = new Brush(GeometrySystem::LoadFromFile(path));

		brush->SetPosition(pos);
		brush->SetRotation(rot);
		brush->SetScale(scale);
		brush->SetLayerID(layerId);
		brush->SetName(name);

		m_Brushes.push_back(brush);

		ObjectLayerManager::Get()->AddToLayer(brush, layerId);
		PhysicsEngine::Get()->AddEntity(brush->GetPhysicalEntity());
		return brush;
	}

	void BrushManager::Remove(Brush* brush)
	{
		auto it = std::find(m_Brushes.begin(), m_Brushes.end(), brush);
		if (it != m_Brushes.end())
		{
			m_Brushes.erase(it);
		}

		Lamp::PhysicsEngine::Get()->RemoveEntity(brush->GetPhysicalEntity());
		delete brush;
	}

	Brush* BrushManager::GetBrushFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		for (auto& brush : m_Brushes)
		{
			Ray ray;
			ray.origin = origin;
			ray.direction = pos;

			if (brush->GetPhysicalEntity()->GetCollider()->IntersectRay(ray).IsIntersecting)
			{
				return brush;
			}
		}

		return nullptr;
	}

	Brush* BrushManager::GetBrushFromPhysicalEntity(Ref<PhysicalEntity>& pEnt)
	{
		for (auto& brush : m_Brushes)
		{
			if (brush->GetPhysicalEntity() == pEnt)
			{
				return brush;
			}
		}

		return nullptr;
	}
}