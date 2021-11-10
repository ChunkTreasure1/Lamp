#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"

#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp
{
	class IBLBuffer : public Framebuffer
	{
	public:
		IBLBuffer(const std::string& path);
		virtual ~IBLBuffer() override;

		void Bind() override;
		void Unbind() override;
		void Resize(const uint32_t width, const uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		void Copy(uint32_t rendererId, const glm::vec2& size, bool depth) override;
		void Invalidate() override {}

		inline const uint32_t GetColorAttachmentID(uint32_t i /* = 0 */) override;
		inline const uint32_t GetDepthAttachmentID() override;
		inline const uint32_t GetRendererID() override { return -1; }

		void ClearAttachment(uint32_t attachmentIndex, int value) override;

		void BindColorAttachment(uint32_t id, uint32_t i) override;
		void BindDepthAttachment(uint32_t id) override;

		FramebufferSpecification& GetSpecification() override { return m_specification; }

	private:
		Ref<Shader> m_EqCubeShader;
		Ref<Shader> m_ConvolutionShader;
		Ref<Shader> m_PrefilterShader;
		Ref<Shader> m_BRDFShader;

		uint32_t m_CubeMapId = 0;
		uint32_t m_HdrTextureId = 0;
		uint32_t m_IrradianceId = 0;
		uint32_t m_RendererId = 0;
		uint32_t m_PrefilterMap = 0;
		uint32_t m_BRDFLUTMap = 0;

		glm::mat4 m_CaptureProjection;
		std::vector<glm::mat4> m_CaptureViews;
		std::array<uint32_t, 2> m_Attachments;

		FramebufferSpecification m_specification;
	};
}