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
				case Lamp::RenderAttributeType::Pass:
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
				case Lamp::RenderAttributeType::Pass:
					break;
				default:
					break;
			}

			return 0;
		}
	}
}