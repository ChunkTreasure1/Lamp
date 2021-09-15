#include "lppch.h"
#include "Brush.h"

#include "Lamp/Level/Level.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	Brush::Brush(Ref<Mesh> model)
		: m_Model(model)
	{
		m_Name = "Brush";
	}

	void Brush::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush::OnRender));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Brush::OnUpdate));
	}

	void Brush::Destroy()
	{
		g_pEnv->pLevel->GetBrushes().erase(m_Id);

		for (auto& layer : g_pEnv->pLevel->GetLayers())
		{
			if (auto it = std::find(layer.Objects.begin(), layer.Objects.end(), this); it != layer.Objects.end())
			{
				layer.Objects.erase(it);
			}
		}

		delete this;
	}

	void Brush::ScaleChanged()
	{
		m_Model->GetBoundingBox().Max = m_Scale * m_Model->GetBoundingBox().StartMax;
		m_Model->GetBoundingBox().Min = m_Scale * m_Model->GetBoundingBox().StartMin;
	}

	Brush* Brush::Create(const std::string& path)
	{
		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(path);
		Brush* brush = new Brush(model);
		brush->SetLayerID(0);

		g_pEnv->pLevel->GetBrushes().emplace(std::make_pair(brush->GetID(), brush));
		g_pEnv->pLevel->GetLayers()[brush->GetLayerID()].Objects.push_back(brush);

		return brush;
	}

	Brush* Brush::Create(const std::string& path, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, uint32_t layerId, const std::string& name)
	{
		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(path);
		Brush* brush = new Brush(model);

		brush->SetPosition(pos);
		brush->SetRotation(rot); 
		brush->SetScale(scale);
		brush->SetLayerID(layerId);
		brush->SetName(name);

		g_pEnv->pLevel->GetBrushes().emplace(std::make_pair(brush->GetID(), brush));
		g_pEnv->pLevel->GetLayers()[brush->GetLayerID()].Objects.push_back(brush);

		return brush;
	}

	Brush* Brush::Duplicate(Brush* main, bool addToLevel)
	{
		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(main->GetModel()->Path);
		Brush* pBrush = new Brush(model);

		if (addToLevel)
		{
			g_pEnv->pLevel->GetBrushes().emplace(std::make_pair(pBrush->m_Id, pBrush));
			g_pEnv->pLevel->GetLayers()[pBrush->GetLayerID()].Objects.push_back(pBrush);
		}
		else
		{
			pBrush->m_Id = main->m_Id;
		}

		pBrush->SetLayerID(main->GetLayerID());
		pBrush->SetPosition(main->GetPosition());
		pBrush->SetRotation(main->GetRotation());
		pBrush->SetScale(main->GetScale());

		std::string name = main->GetName();
		if (auto pos = name.find_last_of("1234567890"); pos != std::string::npos)
		{
			int currIndex = stoi(name.substr(pos, 1));
			currIndex++;
			name.replace(pos, 1, std::to_string(currIndex));
		}

		pBrush->SetName(name);
	
		return pBrush;
	}

	Brush* Brush::Get(uint32_t id)
	{
		if (auto& it = g_pEnv->pLevel->GetBrushes().find(id); it != g_pEnv->pLevel->GetBrushes().end())
		{
			return g_pEnv->pLevel->GetBrushes().at(id);
		}

		return nullptr;
	}

	bool Brush::OnRender(AppRenderEvent& e)
	{
		if (g_pEnv->ShouldRenderBB && e.GetPassInfo().type == PassType::Forward)
		{
			m_Model->RenderBoundingBox(m_ModelMatrix);
		}
		
		if (e.GetPassInfo().type == PassType::Forward)
		{
			return true;
		}

		bool forward = e.GetPassInfo().type == PassType::Selection ? true : false;

		m_Model->Render(m_Id, m_ModelMatrix, forward);

		return true;
	}

	bool Brush::OnUpdate(AppUpdateEvent& e)
	{
		return false;
	}
}