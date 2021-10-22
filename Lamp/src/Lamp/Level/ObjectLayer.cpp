#include "lppch.h"
#include "ObjectLayer.h"

#include "Lamp/Objects/Object.h"

namespace Lamp
{
	ObjectLayer::~ObjectLayer()
	{
		for (int i = Objects.size() - 1; i >= 0; --i)
		{
			Objects[i]->Destroy();
		}
	}
}