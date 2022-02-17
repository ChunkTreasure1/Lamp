#include "lppch.h"
#include "Framebuffer.h"

#include "Lamp/Rendering/Swapchain.h"

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
		: m_specification(spec)
	{
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

			for (auto& attachmentSpec : m_specification.attachments.Attachments)
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

		LP_CORE_ASSERT(m_specification.attachments.Attachments.size(), "Attachment count must be greater than zero!");
		LP_CORE_INFO("VulkanFramebuffer: Creating!");
		Resize(m_width, m_height);
	}

	Framebuffer::~Framebuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto swapchain = Application::Get().GetWindow().GetSwapchain();

		swapchain->UnregisterResizeCallback(this);
		vkDeviceWaitIdle(device->GetHandle());

		if (!m_specification.swapchainTarget)
		{
			vkDestroyFramebuffer(device->GetHandle(), m_framebuffer, nullptr);
			vkDestroyRenderPass(device->GetHandle(), m_renderPass, nullptr);
		}

		LP_CORE_INFO("VulkanFramebuffer: Destroyed");
	}

	void Framebuffer::Bind()
	{
	}

	void Framebuffer::Unbind()
	{
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
			auto& swapchain = Application::Get().GetWindow().GetSwapchain();
			m_renderPass = swapchain->GetRenderPass();

			m_clearValues.clear();
			m_clearValues.emplace_back().color = { 0.f, 0.f, 0.f, 1.f };
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

		if (m_framebuffer)
		{
			VkFramebuffer framebuffer = m_framebuffer;
			vkDeviceWaitIdle(device);
			vkDestroyFramebuffer(device, framebuffer, nullptr);

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
		}

		VulkanAllocator allocator;

		std::vector<VkAttachmentDescription> attachmentDescriptons;
		std::vector<VkAttachmentReference> colorAttachmentReferences;
		VkAttachmentReference depthAttachmentReference;

		m_clearValues.resize(m_specification.attachments.Attachments.size());

		bool createImages = m_attachmentImages.empty();

		uint32_t attachmentIndex = 0;
		for (auto attachmentSpec : m_specification.attachments.Attachments)
		{
			if (Utils::IsDepthFormat(attachmentSpec.textureFormat))
			{
				auto vulkanImage = m_depthAttachmentImage;

				auto& spec = const_cast<ImageSpecification&>(vulkanImage->GetSpecification());
				spec.width = m_width;
				spec.height = m_height;
				m_depthAttachmentImage->Invalidate(nullptr);

				VkAttachmentDescription& attachmentDesc = attachmentDescriptons.emplace_back();
				attachmentDesc.flags = 0;
				attachmentDesc.format = Utils::LampFormatToVulkanFormat(attachmentSpec.textureFormat);
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.loadOp = Utility::LampLoadToVulkanLoadOp(attachmentSpec.clearMode);
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				depthAttachmentReference = { attachmentIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

				m_clearValues[attachmentIndex].depthStencil = { 1.f, 0 };
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
				}
				else
				{
					Ref<Image2D> image = m_attachmentImages[attachmentIndex];
					ImageSpecification& spec = const_cast<ImageSpecification&>(image->GetSpecification());

					spec.width = m_width;
					spec.height = m_height;

					colorAttachment = image;
					colorAttachment->Invalidate(nullptr);
				}

				VkAttachmentDescription& attachmentDescripton = attachmentDescriptons.emplace_back();
				attachmentDescripton.flags = 0;
				attachmentDescripton.format = Utils::LampFormatToVulkanFormat(attachmentSpec.textureFormat);
				attachmentDescripton.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescripton.loadOp = Utility::LampLoadToVulkanLoadOp(attachmentSpec.clearMode); // TODO: add other ops
				attachmentDescripton.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescripton.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescripton.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDescripton.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescripton.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				const auto& clearColor = m_specification.clearColor;
				m_clearValues[attachmentIndex].color = { {clearColor.r, clearColor.g, clearColor.b, clearColor.a} };
				colorAttachmentReferences.emplace_back(VkAttachmentReference{ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachmentIndex++;
		}

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();

		if (m_depthAttachmentImage)
		{
			subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		}

		std::vector<VkSubpassDependency> dependencies;

		if (m_attachmentImages.size())
		{
			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = 0;
				dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		if (m_depthAttachmentImage)
		{
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
				depedency.dstSubpass = 0;
				depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = 0;
				depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
				depedency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		//Create the render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptons.size());
		renderPassInfo.pAttachments = attachmentDescriptons.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		LP_VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass));

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

		//Create framebuffer
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_width;
		framebufferInfo.height = m_height;
		framebufferInfo.layers = 1;

		LP_VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer));
	}

	void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
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