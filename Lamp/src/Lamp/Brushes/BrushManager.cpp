#include "lppch.h"
#include "BrushManager.h"

namespace Lamp
{
	BrushManager BrushManager::s_Manager;

	BrushManager::BrushManager()
		: m_pShader(nullptr)
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
		if (m_pShader == nullptr)
		{
			m_pShader = Shader::Create("engine/shaders/shader_vs.glsl", "engine/shaders/shader_fs.glsl");
		}

		auto brush = new Brush(path);
		brush->GetModel().GetMaterial().SetShader(m_pShader);

		brush->GetModel().GetMaterial().SetDiffuse(Texture2D::Create("engine/textures/container_diff.png"));
		brush->GetModel().GetMaterial().SetSpecular(Texture2D::Create("engine/textures/container_spec.png"));

		m_Brushes.push_back(brush);
	
		return m_Brushes[m_Brushes.size() - 1];
	}

	Brush* BrushManager::Create(const std::string& path, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		auto brush = new Brush(path);
		
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

	Brush* BrushManager::GetBrushFromPoint(const glm::vec2& pos)
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