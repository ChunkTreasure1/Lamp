#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp
{
	class IBLBuffer : public Framebuffer
	{
	public:
		IBLBuffer(const std::string& path);
		virtual ~IBLBuffer() override;

		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(const uint32_t width, const uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual void Copy(uint32_t rendererId, const glm::vec2& size, bool depth) override;

		virtual inline const uint32_t GetColorAttachmentID(uint32_t i /* = 0 */) override;
		virtual inline const uint32_t GetDepthAttachmentID() override;
		virtual inline const uint32_t GetRendererID() override { return -1; }

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual void BindColorAttachment(uint32_t id, uint32_t i);
		virtual void BindDepthAttachment(uint32_t id);

		virtual const FramebufferSpecification& GetSpecification() const override { return FramebufferSpecification(); }

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
		std::array<uint32_t, 3> m_Attachments;
	};
}