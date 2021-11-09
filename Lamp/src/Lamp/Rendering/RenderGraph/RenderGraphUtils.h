#pragma once

#include "RenderNode.h"
#include <imgui.h>

namespace Lamp
{
	namespace Utils
	{
		static uint32_t GetTypeColor(RenderAttributeType type)
		{
			switch (type)
			{
				case Lamp::RenderAttributeType::Texture: return IM_COL32(62, 189, 100, 255);
				case Lamp::RenderAttributeType::Framebuffer: return IM_COL32(150, 28, 17, 255);
				case Lamp::RenderAttributeType::DynamicUniform: return IM_COL32(153, 64, 173, 255);
				case Lamp::RenderAttributeType::Pass: return IM_COL32(74, 58, 232, 255);
					break;
				default:
					break;
			}

			return 0;
		}

		static uint32_t GetTypeHoverColor(RenderAttributeType type)
		{
			switch (type)
			{
				case Lamp::RenderAttributeType::Texture: return IM_COL32(100, 181, 124, 255);
				case Lamp::RenderAttributeType::Framebuffer: return IM_COL32(179, 53, 41, 255);
				case Lamp::RenderAttributeType::DynamicUniform: return IM_COL32(159, 94, 173, 255);
				case Lamp::RenderAttributeType::Pass: return IM_COL32(64, 97, 255, 255);
					break;
				default:
					break;
			}

			return 0;
		}

		template<typename T, class Container>
		static T* GetSpecificationById(Container& container, GraphUUID id)
		{
			if (auto it = std::find_if(container.begin(), container.end(), [&id](const T& item) { return item.id == id; }); it != container.end())
			{
				return &*it;
			}

			return nullptr;
		}
	}
}