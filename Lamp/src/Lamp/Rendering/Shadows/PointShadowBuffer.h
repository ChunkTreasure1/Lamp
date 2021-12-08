#pragma once

#include <functional>

#include "Lamp/Rendering/Buffers/Framebuffer.h"

namespace Lamp
{
	class PointShadowBuffer : Framebuffer
	{
	public:
		PointShadowBuffer(const FramebufferSpecification& spec);
		virtual ~PointShadowBuffer() override;

		void Bind() override;
		void Unbind() override;
		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override { return -1; }
		void Copy(uint32_t rendererId, const glm::vec2& size, bool depth) override {}

		inline const std::vector<glm::mat4>& GetTransforms() { return m_Transforms; }

		Ref<Image2D> GetColorAttachment(uint32_t index) const override { return nullptr; }
		Ref<Image2D> GetDepthAttachment() const override { return nullptr; }

		inline const uint32_t GetColorAttachmentID(uint32_t i) override { return 0; }
		inline const uint32_t GetDepthAttachmentID() override { return m_TextureID; }
		inline const uint32_t GetRendererID() override { return m_RendererID; }

		void ClearAttachment(uint32_t attachmentIndex, int value) override {}

		void BindColorAttachment(uint32_t id /* = 0 */, uint32_t i /* = 0 */) override;
		void BindDepthAttachment(uint32_t id /* = 0 */) override;

		FramebufferSpecification& GetSpecification() override { return FramebufferSpecification(); }

		void SetPosition(const glm::vec3& pos);
		inline const glm::vec3& GetPosition() { return m_Position; }

		inline float& GetNearPlane() { return m_NearPlane; }
		inline float& GetFarPlane() { return m_FarPlane; }

		void UpdateProjection();

	private:
		void Invalidate() override;

	private:
		float m_NearPlane = 0.1f;
		float m_FarPlane = 25.f;

		std::vector<glm::mat4> m_Transforms;
		glm::mat4 m_Projection = glm::mat4(1.f);

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };

		uint32_t m_RendererID;
		uint32_t m_TextureID;

		FramebufferSpecification m_Specification;
	};
}