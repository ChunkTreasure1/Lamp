#include "lppch.h"
#include "GlobalEnvironment.h"

RenderUtils::~RenderUtils()
{
	m_PointLights.clear();
}

void RenderUtils::RegisterPointLight(Lamp::PointLight* light)
{
	m_PointLights.push_back(light);
}

bool RenderUtils::UnregisterPointLight(Lamp::PointLight* light)
{
	for (int i = 0; i < m_PointLights.size(); i++)
	{
		if (m_PointLights[i]->Id == light->Id)
		{
			m_PointLights.erase(m_PointLights.begin() + i);
			return true;
		}
	}

	return false;
}

void RenderUtils::RegisterMeshComponent(uint32_t id, Lamp::MeshComponent* mesh)
{
	m_MeshComponents.insert(std::make_pair(id, mesh));
}

bool RenderUtils::UnegisterMeshComponent(uint32_t id)
{
	if (m_MeshComponents.find(id) != m_MeshComponents.end())
	{
		m_MeshComponents.erase(id);
		return true;
	}
	return false;
}
