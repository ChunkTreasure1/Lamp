#pragma once

#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include "Platform/Vulkan/VulkanImage2D.h"

namespace Lamp
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer() override;

		void Bind() override;
		void Unbind() override;

		void Resize(const uint32_t width, const uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		void Copy(uint32_t rendererId, const glm::vec2& size, bool depth /* = false */) override;
		void Invalidate() override;

		inline VkRenderPass GetRenderPass() const { return m_renderPass; }
		inline VkFramebuffer GetFramebuffer() const { return m_framebuffer; }
		inline const std::vector<VkClearValue>& GetClearValues() { return m_clearValues; }

		void ClearAttachment(uint32_t attachmentIndex, int value) override;

		Ref<Image2D> GetColorAttachment(uint32_t index) const override;
		Ref<Image2D> GetDepthAttachment() const override;

		const uint32_t GetColorAttachmentID(uint32_t i /* = 0 */) override;
		const uint32_t GetDepthAttachmentID() override;
		const uint32_t GetRendererID() override;

		void BindColorAttachment(uint32_t id /* = 0 */, uint32_t i /* = 0 */) override;
		void BindDepthAttachment(uint32_t id /* = 0 */) override;

		FramebufferSpecification& GetSpecification() override;

	private:
		FramebufferSpecification m_specification;
	
		uint32_t m_width;
		uint32_t m_height;

		VkRenderPass m_renderPass;
		VkFramebuffer m_framebuffer = nullptr;

		Ref<VulkanImage2D> m_depthAttachmentImage;
		std::vector<Ref<VulkanImage2D>> m_attachmentImages;

		std::vector<VkClearValue> m_clearValues;
	};
}