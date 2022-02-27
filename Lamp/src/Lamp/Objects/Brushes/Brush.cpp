#include "lppch.h"
#include "Brush.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Level/LevelManager.h"

#include "Lamp/AssetSystem/ResourceCache.h"

#include "Lamp/Mesh/MeshInstance.h"

namespace Lamp
{
	Brush::Brush(Ref<MeshInstance> mesh)
		: m_mesh(mesh)
	{
		m_name = "Brush";
		m_boundingMesh = MeshInstance::Create(ResourceCache::GetAsset<Mesh>("assets/meshes/base/sphere.lgf"));
	}

	void Brush::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush::OnRender));
	}

	void Brush::Destroy()
	{
		if (!LevelManager::IsLevelLoaded())
		{
			LP_CORE_ERROR("Trying to destroy brush when no level was loaded!");
			return;
		}

		auto level = LevelManager::GetActive();

		level->RemoveFromLayer(this);
		level->GetBrushes().erase(m_id);

		delete this;
	}

	Brush* Brush::Create(const std::filesystem::path& path, bool addToLevel)
	{
		if (!LevelManager::IsLevelLoaded() && addToLevel)
		{
			LP_CORE_ERROR("Trying to create brush when no level was loaded!");
			return nullptr;
		}

		Brush* brush = new Brush(MeshInstance::Create(ResourceCache::GetAsset<Mesh>(path)));
		brush->SetLayerID(0);

		if (addToLevel)
		{
			auto level = LevelManager::GetActive();

			level->GetBrushes().emplace(std::make_pair(brush->GetID(), brush));
			level->AddToLayer(brush);
		}

		return brush;
	}

	Brush* Brush::Create(const std::filesystem::path& path, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, uint32_t layerId, const std::string& name)
	{
		if (!LevelManager::IsLevelLoaded())
		{
			LP_CORE_ERROR("Trying to create brush when no level was loaded!");
			return nullptr;
		}

		Brush* brush = new Brush(MeshInstance::Create(ResourceCache::GetAsset<Mesh>(path)));

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
		if (!LevelManager::IsLevelLoaded())
		{
			LP_CORE_ERROR("Trying to duplicate brush when no level was loaded!");
			return nullptr;
		}

		Brush* pBrush = new Brush(MeshInstance::Create(main->GetMesh()->GetSharedMesh()));

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
		if (!LevelManager::IsLevelLoaded())
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
			m_mesh->Render(GetTransform(), m_id);
		}

		//TODO: readd bound rendering
		//if (g_pEnv->shouldRenderBB)
		//{
		//	auto transform = GetTransform();

		//	glm::vec3 globalScale = { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]) };

		//	for (const auto& subMesh : m_mesh->GetSubMeshes())
		//	{
		//		auto& bv = subMesh->GetBoundingVolume();
		//		const glm::vec3 globalCenter = transform * glm::vec4(bv.GetCenter(), 1.f);

		//		const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);
		//		const float radius = bv.GetRadius();
		//		const float scale = radius * maxScale * 0.5f;

		//		glm::mat4 newTransform = glm::translate(glm::mat4(1.f), globalCenter) * glm::scale(glm::mat4(1.f), { scale, scale, scale });

		//		m_boundingMesh->Render(m_id, newTransform);
		//	}
		//}

		return false;
	}
}