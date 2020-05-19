#include "lppch.h"
#include "BrushManager.h"

#include "Lamp/Meshes/GeometrySystem.h"
#include "Lamp/Physics/Ray.h"
#include "Lamp/Entity/Base/Physical/PhysicalEntity.h"

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
	
		return m_Brushes[m_Brushes.size() - 1];
	}

	Brush* BrushManager::Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		auto brush = new Brush(GeometrySystem::LoadFromFile(path));
		
		brush->SetPosition(pos);
		brush->SetRotation(rot);
		brush->SetScale(scale);

		m_Brushes.push_back(brush);

		return brush;
	}

	void BrushManager::Remove(const Brush* brush)
	{
		auto it = std::find(m_Brushes.begin(), m_Brushes.end(), brush);
		for (auto someBrush : m_Brushes)
		{
			if (someBrush == brush)
			{
				delete someBrush;
				break;
			}
		}

		if (it != m_Brushes.end())
		{
			m_Brushes.erase(it);
		}
	}

	void BrushManager::OnEvent(Event& e)
	{
		if (e.GetEventType() == EventType::AppRender)
		{
			for (auto brush : m_Brushes)
			{
				brush->Render();
			}
		}
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
}