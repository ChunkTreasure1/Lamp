#pragma once

#include <memory>

namespace Lamp
{
	class ShadowMap
	{
	public:
		ShadowMap(const uint32_t width, const uint32_t height);

		void Bind();
		void Unbind();

		static Ref<ShadowMap> Create(const uint32_t width, const uint32_t height);

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;

		uint32_t m_Width;
		uint32_t m_Height;
	};
}