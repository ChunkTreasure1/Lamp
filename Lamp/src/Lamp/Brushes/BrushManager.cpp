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
		//auto brush = new Brush(GeometrySystem::ImportModel(path));
		//brush->GetModel().GetMaterial().SetShader(m_pShader);

		//brush->GetModel().GetMaterial().SetDiffuse(Texture2D::Create("engine/textures/container_diff.png"));
		//brush->GetModel().GetMaterial().SetSpecular(Texture2D::Create("engine/textures/container_spec.png"));

		//std::string s = path;
		//s = s.substr(0, s.find_last_of('.'));
		//s += ".lgf";

		//GeometrySystem::SaveToPath(brush->GetModel(), s);

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

		return m_Brushes[m_Brushes.size() - 1];
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
				brush->Draw();
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