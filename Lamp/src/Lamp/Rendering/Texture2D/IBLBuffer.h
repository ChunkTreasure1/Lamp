#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Lamp
{
	class IBLBuffer
	{
	public:
		IBLBuffer(const std::string& path);

		void Bind();
		inline const uint32_t GetTextureID() { return m_CubeMapId; }
		inline const uint32_t GetPrefilterID() { return m_PrefilterMap; }
		inline const uint32_t GetBRDFLUTID() { return m_TestTexture->GetID(); }
		inline const uint32_t GetIrradianceID() { return m_IrradianceId; }

	private:
		Ref<Shader> m_EqCubeShader;
		Ref<Shader> m_ConvolutionShader;
		Ref<Shader> m_PrefilterShader;
		Ref<Shader> m_BRDFShader;

		Ref<Texture2D> m_TestTexture;

		uint32_t m_CubeMapId = 0;
		uint32_t m_HdrTextureId = 0;
		uint32_t m_IrradianceId = 0;
		uint32_t m_RendererId = 0;
		uint32_t m_PrefilterMap = 0;
		uint32_t m_BRDFLUTMap = 0;

		glm::mat4 m_CaptureProjection;
		std::vector<glm::mat4> m_CaptureViews;
	};
}