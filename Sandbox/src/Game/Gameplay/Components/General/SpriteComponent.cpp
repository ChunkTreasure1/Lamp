#include "lppch.h"
#include "SpriteComponent.h"

#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	bool SpriteComponent::s_Registered = LP_REGISTER_COMPONENT(SpriteComponent);
}