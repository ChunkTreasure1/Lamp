#include <Lamp/Input/ResourceManager.h>

#include "SpriteComponent.h"

namespace LampEntity
{
	void SpriteComponent::Initialize()
	{
		m_Texture = Lamp::ResourceManager::GetTexture(m_Path);
	}
}