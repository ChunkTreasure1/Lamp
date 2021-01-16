#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class SkyboxDraw
	{
	public:
		SkyboxDraw(const std::string& path);

		void Draw(const glm::mat4& view);
		void RenderCube();

	private:
		Ref<Shader> m_pEqCubeShader;
		Ref<Shader> m_pSkyboxShader;

		uint32_t cubeVAO = 0;
		uint32_t cubeVBO = 0;
		uint32_t envCubemap = 0;
	};
}