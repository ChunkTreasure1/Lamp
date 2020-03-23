#include "lppch.h"
#include "BrushManager.h"

namespace Lamp
{
	BrushManager BrushManager::s_Manager;

	BrushManager::BrushManager()
	{
	}

	BrushManager::~BrushManager()
	{
		for (size_t i = 0; i < m_Brushes.size(); i++)
		{
			//FIX
			//delete m_Brushes[i];
		}

		m_Brushes.clear();
	}

	void BrushManager::Draw()
	{
		for (auto& brush : m_Brushes)
		{
			brush->Draw();
		}
	}

	Brush2D* BrushManager::Create(const std::string& spritePath)
	{
		auto brush = new Brush2D(spritePath);
		m_Brushes.push_back(brush);
	
		return m_Brushes[m_Brushes.size() - 1];
	}

	Brush2D* BrushManager::Create(const std::string& spritePath, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, bool shouldCollide)
	{
		auto brush = new Brush2D(spritePath);
		
		brush->SetPosition(pos);
		brush->SetRotation(rot);
		brush->SetScale(scale);
		brush->SetShouldCollider(shouldCollide);

		m_Brushes.push_back(brush);

		return m_Brushes[m_Brushes.size() - 1];
	}

	void BrushManager::Remove(const Brush2D* brush)
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

	Brush2D* BrushManager::GetBrushFromPoint(const glm::vec2& pos)
	{
		for (auto& brush : m_Brushes)
		{
			glm::vec4 rect(brush->GetPosition().x, brush->GetPosition().y, brush->GetScale().x, brush->GetScale().y);

			if ((pos.x > (rect.x - (rect.z / 2))) &&
				pos.x < (rect.x + (rect.z / 2)) &&
				
				pos.y > (rect.y - (rect.w / 2)) &&
				pos.y < (rect.y + (rect.w / 2)))
			{
				return brush;
			}
		}

		return nullptr;
	}
}