#include "lppch.h"
#include "Brush.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Level/LevelManager.h"

#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	Brush::Brush(Ref<Mesh> model)
		: m_Mesh(model)
	{
		m_name = "Brush";
	}

	void Brush::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush::OnRender));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Brush::OnUpdate));
	}

	void Brush::Destroy()
	{
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to destroy brush when no level was loaded!");
			return;
		}

		auto level = LevelManager::GetActive();

		level->RemoveFromLayer(this);
		level->GetBrushes().erase(m_id);

		delete this;
	}

	Brush* Brush::Create(const std::filesystem::path& path)
	{
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to create brush when no level was loaded!");
			return nullptr;
		}

		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(path);
		Brush* brush = new Brush(model);
		brush->SetLayerID(0);

		auto level = LevelManager::GetActive();

		level->GetBrushes().emplace(std::make_pair(brush->GetID(), brush));
		level->AddToLayer(brush);

		return brush;
	}

	Brush* Brush::Create(const std::filesystem::path& path, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, uint32_t layerId, const std::string& name)
	{
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to create brush when no level was loaded!");
			return nullptr;
		}

		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(path);
		Brush* brush = new Brush(model);

		brush->SetPosition(pos);
		brush->SetRotation(rot); 
		brush->SetScale(scale);
		brush->SetLayerID(layerId);
		brush->SetName(name);

		auto level = LevelManager::GetActive();

		level->GetBrushes().emplace(std::make_pair(brush->GetID(), brush));
		level->AddToLayer(brush);

		return brush;
	}

	Brush* Brush::Duplicate(Brush* main, bool addToLevel)
	{
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to duplicate brush when no level was loaded!");
			return nullptr;
		}

		Ref<Mesh> model = ResourceCache::GetAsset<Mesh>(main->GetModel()->Path);
		Brush* pBrush = new Brush(model);

		if (addToLevel)
		{
			auto level = LevelManager::GetActive();

			level->GetBrushes().emplace(std::make_pair(pBrush->m_id, pBrush));
			level->AddToLayer(pBrush);
		}
		else
		{
			pBrush->m_id = main->m_id;
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
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to get brush when no level was loaded!");
			return nullptr;
		}

		auto level = LevelManager::GetActive();

		if (auto& it = level->GetBrushes().find(id); it != level->GetBrushes().end())
		{
			return level->GetBrushes().at(id);
		}

		return nullptr;
	}

	bool Brush::OnRender(AppRenderEvent& e)
	{	
		if (m_isActive)
		{
			m_Mesh->Render(m_id, GetTransform());
		}

		return false;
	}

	bool Brush::OnUpdate(AppUpdateEvent& e)
	{
		return false;
	}

	bool Brush::OnScaleChanged(ObjectScaleChangedEvent& e)
	{
		m_Mesh->GetBoundingBox().Max = m_scale * m_Mesh->GetBoundingBox().StartMax;
		m_Mesh->GetBoundingBox().Min = m_scale * m_Mesh->GetBoundingBox().StartMin;

		return false;
	}
}