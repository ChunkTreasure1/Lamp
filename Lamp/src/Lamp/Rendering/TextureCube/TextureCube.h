#pragma once

#include <string>
#include "Lamp/Core/Core.h"
#include <memory>

namespace Lamp
{
	class TextureCube
	{
	public:
		TextureCube(const std::vector<std::string>& paths);

		void Bind();
		void Unbind();

		inline const uint32_t GetID() const { return m_RendererID; }

	public:
		static Ref<TextureCube> Create(const std::vector<std::string>& paths) { return CreateRef<TextureCube>(paths); }
	
		uint32_t m_RendererID;
		std::vector<std::string> m_Paths;
	};
}