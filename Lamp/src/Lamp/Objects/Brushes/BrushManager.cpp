#include "lppch.h"
#include "BrushManager.h"

#include "Lamp/Meshes/GeometrySystem.h"
#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	BrushManager* BrushManager::s_Manager = nullptr;

	BrushManager::BrushManager()
	{
		s_Manager = this;
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
		auto brush = new Brush(GeometrySystem::LoadFromFile(path));
		brush->SetLayerID(0);

		m_Brushes.push_back(brush);

		ObjectLayerManager::Get()->AddToLayer(brush, 0);
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
		return brush;
	}

	void BrushManager::Remove(Brush* brush)
	{
		auto it = std::find(m_Brushes.begin(), m_Brushes.end(), brush);
		if (it != m_Brushes.end())
		{
			m_Brushes.erase(it);
		}
	}

	void BrushManager::Remove(Brush2D* brush)
	{
		auto it = std::find(m_2DBrushes.begin(), m_2DBrushes.end(), brush);
		if (it != m_2DBrushes.end())
		{
			m_2DBrushes.erase(it);
		}

	}

	Brush* BrushManager::GetBrushFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		//Brush* brush = dynamic_cast<Brush*>(ObjectLayerManager::Get()->GetObjectFromPoint(pos, origin));
		//if (brush)
		//{
		//	return brush;
		//}

		return nullptr;
	}

	Brush2D* BrushManager::GetBrush2DFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		//Brush2D* brush = dynamic_cast<Brush2D*>(ObjectLayerManager::Get()->GetObjectFromPoint(pos, origin));
		//if (brush)
		//{
		//	return brush;
		//}

		return nullptr;
	}
}