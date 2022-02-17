#include "lppch.h"

#include "Renderer2D.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Swapchain.h"

#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/CommandBuffer.h"

#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Lamp
{
	void Renderer2D::Initialize()
	{
		CreateStorage();
		SetupRenderPipelines();
	}

	void Renderer2D::Shutdown()
	{
	}

	void Renderer2D::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		ResetBatchData();
	}

	void Renderer2D::End()
	{
		LP_PROFILE_FUNCTION();
	
		uint32_t dataSize = (uint8_t*)m_storage->quadVertexBufferPtr - (uint8_t*)m_storage->quadVertexBufferBase;
		m_storage->quadVertexBuffer->SetData(m_storage->quadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id)
	{
		LP_PROFILE_FUNCTION();

		RenderCommandData cmd;
		cmd.transform = transform;
		cmd.id = id;
		cmd.color = color;
		cmd.texture = texture;

		m_submitBufferPointer->drawCalls.emplace_back(cmd);
	}

	void Renderer2D::SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color)
	{
		LP_PROFILE_FUNCTION();
	}

	void Renderer2D::DrawQuad(const RenderCommandData& cmd)
	{
		if (m_storage->quadIndexCount >= m_storage->maxIndices)
		{
			StartNewBatch();
		}

		uint32_t textureIndex = 0;

		for (uint32_t i = 1; i < m_storage->textureSlotIndex; i++)
		{
			if (m_storage->textureSlots[i].get() == cmd.texture.get())
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			textureIndex = m_storage->textureSlotIndex;
			m_storage->textureSlots[textureIndex] = cmd.texture;
			m_storage->textureSlotIndex++;
		}

		m_storage->quadVertexBufferPtr->position = cmd.transform * m_storage->quadVertexPositions[0];
		m_storage->quadVertexBufferPtr->color = cmd.color;
		m_storage->quadVertexBufferPtr->textureCoords = { 0.f, 0.f };
		m_storage->quadVertexBufferPtr->textureId = textureIndex;
		m_storage->quadVertexBufferPtr++;

		m_storage->quadVertexBufferPtr->position = cmd.transform * m_storage->quadVertexPositions[1];
		m_storage->quadVertexBufferPtr->color = cmd.color;
		m_storage->quadVertexBufferPtr->textureCoords = { 1.f, 0.f };
		m_storage->quadVertexBufferPtr->textureId = textureIndex;
		m_storage->quadVertexBufferPtr++;

		m_storage->quadVertexBufferPtr->position = cmd.transform * m_storage->quadVertexPositions[2];
		m_storage->quadVertexBufferPtr->color = cmd.color;
		m_storage->quadVertexBufferPtr->textureCoords = { 1.f, 1.f };
		m_storage->quadVertexBufferPtr->textureId = textureIndex;
		m_storage->quadVertexBufferPtr++;

		m_storage->quadVertexBufferPtr->position = cmd.transform * m_storage->quadVertexPositions[3];
		m_storage->quadVertexBufferPtr->color = cmd.color;
		m_storage->quadVertexBufferPtr->textureCoords = { 0.f, 1.f };
		m_storage->quadVertexBufferPtr->textureId = textureIndex;
		m_storage->quadVertexBufferPtr++;

		m_storage->quadIndexCount += 6;
	}

	void Renderer2D::SetupRenderPipelines()
	{
		//Quad pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("quad");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawSkybox = false;
			pipelineSpec.drawTerrain = false;
			pipelineSpec.debugName = "Quad";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float4, "a_Color" },
				{ ElementType::Float2, "a_TexCoords" },
				{ ElementType::Int, "a_TexId" },
				{ ElementType::Int, "a_Id" },
			};

			m_storage->quadPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Line pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("line");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::LineList;
			pipelineSpec.drawSkybox = false;
			pipelineSpec.drawTerrain = false;
			pipelineSpec.debugName = "Line";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
			};

			m_storage->linePipeline = RenderPipeline::Create(pipelineSpec);
		}
	}

	void Renderer2D::CreateStorage()
	{
		m_storage = CreateScope<Renderer2DStorage>();

		/////Quad/////
		m_storage->quadVertexBuffer = VertexBuffer::Create(m_storage->maxVertices * sizeof(QuadVertex));
		m_storage->quadVertexBufferBase = new QuadVertex[m_storage->maxVertices];
		m_storage->textureSlots = new Ref<Texture2D>[Renderer::Get().GetCapabilities().maxShaderTexturesArray];

		uint32_t* quadIndices = new uint32_t[m_storage->maxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_storage->maxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		m_storage->quadIndexBuffer = IndexBuffer::Create(quadIndices, m_storage->maxIndices);

		m_storage->quadVertexPositions[0] = { -0.5f, -0.5f, 0.f, 1.f };
		m_storage->quadVertexPositions[0] = { 0.5f, -0.5f, 0.f, 1.f };
		m_storage->quadVertexPositions[0] = { 0.5f,  0.5f, 0.f, 1.f };
		m_storage->quadVertexPositions[0] = { -0.5f,  0.5f, 0.f, 1.f };

		delete[] quadIndices;
		//////////////

		/////Textures/////
		m_storage->whiteTexture = Renderer::Get().GetDefaults().whiteTexture;
	}

	void Renderer2D::AllocateAndUpdateDescriptors()
	{
		LP_PROFILE_FUNCTION();

		//Quad
		{
			auto shader = m_storage->quadPipeline->GetSpecification().shader;
			auto quadDescriptorLayout = shader->GetDescriptorSetLayout(0);
			auto& shaderDescriptorSet = shader->GetDescriptorSets()[0];

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &quadDescriptorLayout;

			m_storage->currentQuadDescriptorSet = Renderer::Get().AllocateDescriptorSet(allocInfo);

			std::vector<VkWriteDescriptorSet> writeDescriptors;
			
			//Textures
			{
				auto it = shaderDescriptorSet.writeDescriptorSets.find("u_Textures");
				if (it != shaderDescriptorSet.writeDescriptorSets.end())
				{
					auto writeDescriptor = it->second;
					writeDescriptor.dstSet = m_storage->currentQuadDescriptorSet;

					std::vector<VkDescriptorImageInfo> textureInfos;
					for (uint32_t i = 0; i < m_storage->textureSlotIndex; i++)
					{
						auto tex = m_storage->textureSlots[i];
						textureInfos.emplace_back(tex->GetDescriptorInfo());
					}

					writeDescriptor.descriptorCount = textureInfos.size();
					writeDescriptor.pImageInfo = textureInfos.data();

					writeDescriptors.emplace_back(writeDescriptor);
				}
			}

			auto device = VulkanContext::GetCurrentDevice();
			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}

	}

	void Renderer2D::ResetBatchData()
	{
		m_storage->quadIndexCount = 0;
		m_storage->quadVertexBufferPtr = m_storage->quadVertexBufferBase;
		m_storage->textureSlotIndex = 1;
	}

	void Renderer2D::StartNewBatch()
	{
		LP_PROFILE_FUNCTION();

		End();
		ResetBatchData();
	}

	void Renderer2D::Flush()
	{
		LP_PROFILE_FUNCTION();

		AllocateAndUpdateDescriptors();

		for (const auto& cmd : m_renderBufferPointer->drawCalls)
		{
			DrawQuad(cmd);
		}

		auto commandBuffer = m_storage->quadPipeline->GetSpecification().isSwapchain ? m_storage->swapchainCommandBuffer : m_storage->renderCommandBuffer;
	
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
		m_storage->quadPipeline->BindDescriptorSet(commandBuffer, m_storage->currentQuadDescriptorSet, 0);

		m_storage->quadVertexBuffer->Bind(commandBuffer);
		m_storage->quadIndexBuffer->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_storage->quadIndexCount, 1, 0, 0, 0);
	}
}