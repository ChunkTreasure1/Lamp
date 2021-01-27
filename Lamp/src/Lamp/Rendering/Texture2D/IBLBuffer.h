#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"

namespace Lamp
{
	class IBLBuffer
	{
	public:
		IBLBuffer(const std::string& path);

		void Bind();
		inline const uint32_t GetTextureID() { return m_CubeMapId; }

	private:
		Ref<Shader> m_EqCubeShader;

		uint32_t m_CubeMapId = 0;
		uint32_t m_HdrTextureId = 0;
		uint32_t m_RendererId = 0;

		glm::mat4 m_CaptureProjection;
		std::vector<glm::mat4> m_CaptureViews;
	};
}