#pragma once

#include <functional>

namespace Lamp
{
	class PointShadowBuffer
	{
	public:
		PointShadowBuffer(uint32_t width, uint32_t height, const glm::vec3& pos);

		void Bind();
		void Update(uint32_t width, uint32_t height);

	private:
		void Invalidate();

	private:
		std::vector<glm::mat4> m_Transforms;
		glm::mat4 m_Projection = glm::mat4(1.f);

		glm::vec3 m_Position;

		uint32_t m_RendererID;
		uint32_t m_TextureID;

		uint32_t m_Width;
		uint32_t m_Height;
	};
}