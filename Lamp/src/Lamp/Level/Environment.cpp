#include "lppch.h"
#include "Environment.h"

namespace Lamp
{
	Environment::~Environment()
	{
		m_pointLights.clear();
		m_directionalLights.clear();
	}

	void Environment::RegisterPointLight(PointLight* light)
	{
		m_pointLights.push_back(light);
	}

	bool Environment::UnregisterPointLight(PointLight* light)
	{
		for (int i = 0; i < m_pointLights.size(); i++)
		{
			if (m_pointLights[i]->id == light->id)
			{
				m_pointLights.erase(m_pointLights.begin() + i);
				return true;
			}
		}

		return false;
	}

	void Environment::RegisterDirectionalLight(DirectionalLight* light)
	{
		m_directionalLights.push_back(light);
	}

	bool Environment::UnregisterDirectionalLight(DirectionalLight* light)
	{
		for (int i = 0; i < m_directionalLights.size(); i++)
		{
			if (m_directionalLights[i]->Id == light->Id)
			{
				m_directionalLights.erase(m_directionalLights.begin() + i);
				return true;
			}
		}

		return false;
	}
}