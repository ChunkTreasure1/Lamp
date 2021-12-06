#include "lppch.h"
#include "VulkanRenderer.h"

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Renderer.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/AssetSystem/MeshImporter.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanRenderPipeline.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

#define ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

namespace Lamp
{
	VulkanRenderer::VulkanRenderer()
	{
		m_rendererStorage = CreateScope<TempRendererStorage>();

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		auto device = VulkanContext::GetCurrentDevice();
		VkResult result = vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPool);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor pool!");
	}

	VulkanRenderer::~VulkanRenderer()
	{
		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());
		vkDestroyDescriptorPool(device->GetHandle(), m_descriptorPool, nullptr);
	}

	void VulkanRenderer::Initialize()
	{
		SetupBuffers();
		m_rendererStorage->mainShader = Shader::Create("engine/shaders/vulkan/vulkanPbr.glsl", true);

		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = m_rendererStorage->mainShader;
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.uniformBufferSets = m_rendererStorage->uniformBufferSet;
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			m_rendererStorage->mainPipeline = RenderPipeline::Create(pipelineSpec);
		}

		MeshImportSettings settings;
		settings.path = "assets/meshes/teddy/teddy.fbx";
		m_rendererStorage->teddy = MeshImporter::ImportMesh(settings);
		m_rendererStorage->teddy->GetMaterial(0)->SetShader(m_rendererStorage->mainShader);

		auto mat = m_rendererStorage->teddy->GetMaterial(0);

		mat->SetTexture("u_Albedo", Texture2D::Create("assets/textures/TeddyTextures/DJTeddy_final_albedo.tga"));
		mat->SetTexture("u_Normal", Texture2D::Create("assets/textures/TeddyTextures/DJTeddy_final_normal.tga"));
		mat->SetTexture("u_MRO", Texture2D::Create("assets/textures/TeddyTextures/DJTeddy_final_mro.tga"));

		m_rendererStorage->commandBuffer = CommandBuffer::Create(m_rendererStorage->mainPipeline);
	}

	void VulkanRenderer::Shutdown()
	{
	}

	void VulkanRenderer::Begin(const Ref<CameraBase> camera)
	{
		m_rendererStorage->commandBuffer->Begin();

		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto framebuffer = std::reinterpret_pointer_cast<VulkanFramebuffer>(m_rendererStorage->mainPipeline->GetSpecification().framebuffer);

		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = framebuffer->GetRenderPass();
		renderPassBegin.framebuffer = framebuffer->GetFramebuffer();
		renderPassBegin.renderArea.offset = { 0, 0 };

		VkExtent2D extent;
		extent.width = framebuffer->GetSpecification().width;
		extent.height = framebuffer->GetSpecification().height;

		renderPassBegin.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearColors;
		clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
		clearColors[1].depthStencil = { 1.f, 0 };
		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(m_rendererStorage->commandBuffer->GetCurrentCommandBuffer()), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		m_rendererStorage->mainPipeline->Bind(currentFrame);
	}

	void VulkanRenderer::End()
	{
		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(m_rendererStorage->commandBuffer->GetCurrentCommandBuffer()));
		m_rendererStorage->commandBuffer->End();
	}

	void VulkanRenderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->mainPipeline);

		m_rendererStorage->teddy->GetMaterial(0)->Bind(m_rendererStorage->mainPipeline, 0);

		m_rendererStorage->meshBuffer.model = glm::scale(glm::mat4(1.f), { 0.01f, 0.01f, 0.01f }) * glm::rotate(glm::mat4(1.f), glm::radians(90.f), { 1.f, 0.f, 0.f }); // transform

		vulkanPipeline->SetPushConstantData(0, &m_rendererStorage->meshBuffer);
		vulkanPipeline->BindDescriptorSets(0);

		for (const auto subMesh : m_rendererStorage->teddy->GetSubMeshes())
		{
			subMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(m_rendererStorage->commandBuffer);
			subMesh->GetVertexArray()->GetIndexBuffer()->Bind(m_rendererStorage->commandBuffer);

			vkCmdDrawIndexed(static_cast<VkCommandBuffer>(m_rendererStorage->commandBuffer->GetCurrentCommandBuffer()), subMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
		}
	}
		
	void VulkanRenderer::SetupBuffers()
	{ 
		m_rendererStorage->cameraBuffer.view = glm::lookAt(glm::vec3{ 2.f, 2.f, 2.f }, glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 0.f, 0.f, 1.f });
		m_rendererStorage->cameraBuffer.projection = glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f);
		m_rendererStorage->cameraBuffer.position = glm::vec4{ 2.f, 2.f, 2.f, 0.f };

		m_rendererStorage->directionalLightBuffer.colorIntensity = glm::vec4{ 1.f };
		m_rendererStorage->directionalLightBuffer.direction = glm::vec4{ 0.5 };

		m_rendererStorage->uniformBufferSet = UniformBufferSet::Create(Renderer::GetCapabilities().framesInFlight);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->cameraBuffer, sizeof(CameraDataBuffer), 0, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->directionalLightBuffer, sizeof(DirectionalLightDataTest), 1, 0);
	}
}