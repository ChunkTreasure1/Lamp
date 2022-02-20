#include "lppch.h"
#include "Framebuffer.h"

#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/CommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanUtility.h"

namespace Lamp
{
	namespace Utils
	{
		static VkFormat LampFormatToVulkanFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: LP_CORE_ASSERT(false, "Framebuffer attachment must have a format!")
				case ImageFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ImageFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
				case ImageFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
				case ImageFormat::R32SI: return VK_FORMAT_R32_SINT;
				case ImageFormat::R32UI: return VK_FORMAT_R32_UINT;
				case ImageFormat::R32F: return VK_FORMAT_R32_SFLOAT;
				case ImageFormat::DEPTH32F: return VK_FORMAT_D32_SFLOAT;
				case ImageFormat::DEPTH24STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
			}

			return (VkFormat)0;
		}
	}

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<Framebuffer>(spec);
	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_specification(spec), m_depthAttachmentImage(nullptr)
	{
		//Reset values
		memset(&m_depthAttachmentInfo, 0, sizeof(VkRenderingAttachmentInfo));

		if (!m_specification.swapchainTarget)
		{
			if (m_specification.width == 0)
			{
				m_width = Application::Get().GetWindow().GetWidth();
				m_height = Application::Get().GetWindow().GetHeight();
			}
			else
			{
				m_width = spec.width;
				m_height = spec.height;
			}

			for (auto& attachmentSpec : m_specification.attachments.attachments)
			{
				if (Utils::IsDepthFormat(attachmentSpec.textureFormat))
				{
					ImageSpecification imageSpec{};
					imageSpec.format = attachmentSpec.textureFormat;
					imageSpec.usage = ImageUsage::Attachment;
					imageSpec.width = m_width;
					imageSpec.height = m_height;
					imageSpec.copyable = m_specification.copyable;
					imageSpec.comparable = m_specification.shadow;

					m_depthAttachmentImage = Image2D::Create(imageSpec);
				}
				else
				{
					ImageSpecification imageSpec{};
					imageSpec.format = attachmentSpec.textureFormat;
					imageSpec.usage = ImageUsage::Attachment;
					imageSpec.width = m_width;
					imageSpec.height = m_height;
					imageSpec.copyable = m_specification.copyable;
					imageSpec.comparable = m_specification.shadow;

					m_attachmentImages.emplace_back(Image2D::Create(imageSpec));
				}
			}
		}
		else
		{
			auto swapchain = Application::Get().GetWindow().GetSwapchain();
			swapchain->RegisterResizeCallback(this, [=](uint32_t width, uint32_t height)
				{
					Resize(width, height);
				});
		}

		LP_CORE_ASSERT(m_specification.attachments.attachments.size(), "Attachment count must be greater than zero!");
		LP_CORE_INFO("VulkanFramebuffer: Creating!");
		Resize(m_width, m_height);
	}

	Framebuffer::~Framebuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto swapchain = Application::Get().GetWindow().GetSwapchain();

		swapchain->UnregisterResizeCallback(this);
		vkDeviceWaitIdle(device->GetHandle());

		LP_CORE_INFO("VulkanFramebuffer: Destroyed");
	}

	void Framebuffer::Bind(Ref<CommandBuffer> commandBuffer)
	{
		//Transition images to color attachment optimal
		for (auto& attachment : m_attachmentImages)
		{
			Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), attachment->GetHandle(), 0,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}

		if (m_depthAttachmentImage)
		{
			Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), m_depthAttachmentImage->GetHandle(), 0,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
		}
	}

	void Framebuffer::Unbind(Ref<CommandBuffer> commandBuffer)
	{
		//Transition images to color attachment optimal
		for (auto& attachment : m_attachmentImages)
		{
			Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), attachment->GetHandle(), 
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				0,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		}

		if (m_depthAttachmentImage)
		{
			Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), m_depthAttachmentImage->GetHandle(),
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				0,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
		}
	}

	void Framebuffer::Resize(const uint32_t width, const uint32_t height)
	{
		m_specification.width = width;
		m_specification.height = height;
		m_width = width;
		m_height = height;

		if (!m_specification.swapchainTarget)
		{
			Invalidate();
		}
		else
		{
		}
	}

	int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		return 0;
	}

	void Framebuffer::Copy(uint32_t rendererId, const glm::vec2& size, bool depth)
	{
	}

	void Framebuffer::Invalidate()
	{
		auto device = VulkanContext::GetCurrentDevice()->GetHandle();

		uint32_t attachmentIndex = 0;
		for (Ref<Image2D> image : m_attachmentImages)
		{
			auto vulkanImage = image;
			image->Release();
			attachmentIndex++;
		}

		if (m_depthAttachmentImage)
		{
			m_depthAttachmentImage->Release();
		}

		VulkanAllocator allocator;

		bool createImages = m_attachmentImages.empty();

		m_colorFormats.clear();
		m_colorAttachmentInfos.clear();
		if (!createImages)
		{
			m_colorFormats.resize(m_attachmentImages.size());
			m_colorAttachmentInfos.resize(m_attachmentImages.size());
		}

		attachmentIndex = 0;
		for (auto attachmentSpec : m_specification.attachments.attachments)
		{
			if (Utils::IsDepthFormat(attachmentSpec.textureFormat))
			{
				auto vulkanImage = m_depthAttachmentImage;

				auto& spec = const_cast<ImageSpecification&>(vulkanImage->GetSpecification());
				spec.width = m_width;
				spec.height = m_height;
				m_depthAttachmentImage->Invalidate(nullptr);

				m_depthFormat = m_depthAttachmentImage->GetFormat();
				m_depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				m_depthAttachmentInfo.imageView = m_depthAttachmentImage->GetImageView();
				m_depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				m_depthAttachmentInfo.loadOp = Utility::LampLoadToVulkanLoadOp(attachmentSpec.clearMode);
				m_depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				m_depthAttachmentInfo.clearValue.depthStencil = { 1.f, 0 };
			}
			else
			{
				Ref<Image2D> colorAttachment;

				if (createImages)
				{
					ImageSpecification spec{};
					spec.format = attachmentSpec.textureFormat;
					spec.usage = ImageUsage::Attachment;
					spec.width = m_width;
					spec.height = m_height;

					colorAttachment = m_attachmentImages.emplace_back(Image2D::Create(spec));
					m_colorFormats.emplace_back(colorAttachment->GetFormat());

					auto& colInfo = m_colorAttachmentInfos.emplace_back();
					colInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colInfo.imageView = colorAttachment->GetImageView();
					colInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
					colInfo.loadOp = Utility::LampLoadToVulkanLoadOp(attachmentSpec.clearMode);
					colInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colInfo.clearValue.color = { m_specification.clearColor.x, m_specification.clearColor.y, m_specification.clearColor.z, m_specification.clearColor.w };
				}
				else
				{
					Ref<Image2D> image = m_attachmentImages[attachmentIndex];
					ImageSpecification& spec = const_cast<ImageSpecification&>(image->GetSpecification());

					spec.width = m_width;
					spec.height = m_height;

					colorAttachment = image;
					colorAttachment->Invalidate(nullptr);

					m_colorFormats[attachmentIndex] = colorAttachment->GetFormat();
					auto& colInfo = m_colorAttachmentInfos[attachmentIndex];
					colInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
					colInfo.imageView = colorAttachment->GetImageView();
					colInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
					colInfo.loadOp = Utility::LampLoadToVulkanLoadOp(attachmentSpec.clearMode);
					colInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colInfo.clearValue.color = { m_specification.clearColor.x, m_specification.clearColor.y, m_specification.clearColor.z, m_specification.clearColor.w };
				}
			}

			attachmentIndex++;
		}

		std::vector<VkImageView> attachments(m_attachmentImages.size());
		for (uint32_t i = 0; i < m_attachmentImages.size(); i++)
		{
			Ref<Image2D> image = m_attachmentImages[i];
			attachments[i] = image->GetImageView();
			LP_CORE_ASSERT(attachments[i], "Image view is null!");
		}

		if (m_depthAttachmentImage)
		{
			Ref<Image2D> image = m_depthAttachmentImage;
			attachments.emplace_back(image->GetImageView());
			LP_CORE_ASSERT(attachments.back(), "Image view is null!");
		}
	}

	void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
	}

	const VkRenderingAttachmentInfo& Framebuffer::GetColorAttachmentInfo(uint32_t index) const
	{
		return m_colorAttachmentInfos[index];
	}

	const VkRenderingAttachmentInfo& Framebuffer::GetDepthAttachmentInfo() const
	{
		return m_depthAttachmentInfo;
	}

	const std::vector<VkFormat>& Framebuffer::GetColorFormats() const
	{
		return m_colorFormats;
	}

	const VkFormat& Framebuffer::GetDepthFormat() const
	{
		return m_depthFormat;
	}

	const std::vector<VkRenderingAttachmentInfo>& Framebuffer::GetColorAttachmentInfos() const
	{
		return m_colorAttachmentInfos;
	}

	Ref<Image2D> Framebuffer::GetColorAttachment(uint32_t index) const
	{
		LP_CORE_ASSERT(index >= 0 && index < m_attachmentImages.size(), "Index out of bounds!");
		return m_attachmentImages[index];
	}

	Ref<Image2D> Framebuffer::GetDepthAttachment() const
	{
		return m_depthAttachmentImage;
	}

	FramebufferSpecification& Framebuffer::GetSpecification()
	{
		return m_specification;
	}
}