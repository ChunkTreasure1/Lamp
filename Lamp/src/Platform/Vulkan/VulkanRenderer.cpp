#include "lppch.h"
#include "VulkanRenderer.h"

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"
#include "Lamp/Rendering/LightBase.h"
#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"

#include "Lamp/AssetSystem/MeshImporter.h"
#include "Lamp/Level/Level.h"
#include "Lamp/Core/Time/ScopedTimer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanRenderPipeline.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanTextureCube.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanRenderComputePipeline.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

#include "Lamp/World/Terrain.h"

#define ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

#define PER_PASS_DESCRIPTOR_SET 0
#define PER_MESH_DESCRIPTOR_SET 1
#define MAX_DIRECTIONAL_LIGHT_SHADOWS 10


namespace Lamp
{
	Renderer* Renderer::s_instance = nullptr;
	RendererCapabilities Renderer::s_capabilities;

	Renderer::Renderer()
	{
		if (s_instance)
		{
			LP_CORE_ASSERT(!s_instance, "Instance already exists! Singleton should not be created more than once!");
		}

		s_instance = this;
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Initialize()
	{
		PreAllocateRenderBuffers();
		AllocateDescriptorPools();
		CreateRendererStorage();

		m_rendererStorage->swapchainCommandBuffer = CommandBuffer::Create(3, true);
		m_rendererStorage->renderCommandBuffer = CommandBuffer::Create(3, false);
		m_rendererStorage->quadMesh = SubMesh::CreateQuad();

		ShaderLibrary::LoadShaders();

		m_materialLibrary = CreateScope<MaterialLibrary>();
		MaterialLibrary::Get().LoadMaterials();

		GenerateBRDF(m_rendererDefaults->brdfFramebuffer);
	}

	void Renderer::Shutdown()
	{
		MaterialLibrary::Get().Shutdown();

		DestroyRendererStorage();
		DestroyDescriptorPools();
		ClearRenderBuffers();
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		//Reset descriptor pool
		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		const uint32_t currentFrame = swapchain->GetCurrentFrame();
		vkResetDescriptorPool(VulkanContext::GetCurrentDevice()->GetHandle(), m_descriptorPools[currentFrame], 0);

		auto commandBuffer = m_rendererStorage->renderCommandBuffer;
		commandBuffer->Begin();

		m_rendererStorage->camera = camera;
		m_statistics.memoryStatistics = GetMemoryUsage();

		PrepareForRender();
	}

	void Renderer::End()
	{
		LP_PROFILE_FUNCTION();

		{
			LP_PROFILE_SCOPE("Command buffer submit")
			auto commandBuffer = m_rendererStorage->renderCommandBuffer;
			commandBuffer->End();
		}

		m_rendererStorage->camera = nullptr;
		m_finalRenderBuffer.drawCalls.clear();
	}

	void Renderer::SwapRenderBuffers()
	{
		LP_PROFILE_FUNCTION();
		std::swap(m_submitBufferPointer, m_renderBufferPointer);
		m_submitBufferPointer->drawCalls.clear();
	}

	void Renderer::BeginPass(Ref<RenderPipeline> pipeline)
	{
		LP_PROFILE_FUNCTION();
		m_rendererStorage->currentRenderPipeline = pipeline;
		UpdatePerPassUniformBuffers(pipeline);

		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto framebuffer = std::reinterpret_pointer_cast<VulkanFramebuffer>(pipeline->GetSpecification().framebuffer);
		auto commandBuffer = m_rendererStorage->renderCommandBuffer;
		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = framebuffer->GetRenderPass();
		renderPassBegin.framebuffer = pipeline->GetSpecification().isSwapchain ? swapchain->GetFramebuffer(currentFrame) : framebuffer->GetFramebuffer();
		renderPassBegin.renderArea.offset = { 0, 0 };

		VkExtent2D extent;
		extent.width = framebuffer->GetSpecification().width;
		extent.height = framebuffer->GetSpecification().height;

		renderPassBegin.renderArea.extent = extent;

		renderPassBegin.clearValueCount = framebuffer->GetClearValues().size();
		renderPassBegin.pClearValues = framebuffer->GetClearValues().data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		AllocatePerPassDescriptors();
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(pipeline);
		vulkanPipeline->BindDescriptorSet(commandBuffer, m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET], PER_PASS_DESCRIPTOR_SET);
	}

	void Renderer::EndPass()
	{
		LP_PROFILE_FUNCTION();
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()));

		m_rendererStorage->currentRenderPipeline = nullptr;
	}

	const GPUMemoryStatistics& Renderer::GetMemoryUsage() const
	{
		return VulkanAllocator::GetStatistics();
	}

	const RendererStatistics& Renderer::GetStatistics() const
	{
		return m_statistics;
	}

	const RendererCapabilities& Renderer::GetCapabilities() const
	{
		return s_capabilities;
	}

	const VulkanRendererStorage& Renderer::GetStorage() const
	{
		return *m_rendererStorage;
	}

	const RendererDefaults& Renderer::GetDefaults() const
	{
		return *m_rendererDefaults;
	}

	void Renderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */)
	{
		LP_PROFILE_FUNCTION();
		m_submitBufferPointer->drawCalls.emplace_back(transform, mesh, material, id);
	}

	void Renderer::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant /* = nullptr */)
	{
		LP_PROFILE_FUNCTION();
		DrawMesh(mesh, material, descriptorSets, pushConstant);
	}

	void Renderer::DrawMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		LP_PROFILE_FUNCTION();
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);

		AllocateDescriptorsForMaterialRendering(material);
		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->currentMeshDescriptorSets);

		const auto& matData = material->GetMaterialData();

		MeshDataBuffer meshData;
		meshData.blendingUseBlending.x = matData.blendingMultiplier;
		meshData.blendingUseBlending.y = matData.useBlending;

		meshData.useAlbedo = matData.useAlbedo;
		meshData.useNormal = matData.useNormal;
		meshData.useMRO = matData.useMRO;
		meshData.useDetailNormal = matData.useDetailNormal;

		meshData.mroColor = matData.mroColor;
		meshData.albedoColor = matData.albedoColor;
		meshData.normalColor = matData.normalColor;

		meshData.id = id;

		vulkanPipeline->SetPushConstantData(commandBuffer, 0, &transform);
		vulkanPipeline->SetPushConstantData(commandBuffer, 1, &meshData);

		mesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		mesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), mesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DrawMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		LP_PROFILE_FUNCTION();
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);
		vulkanPipeline->BindDescriptorSets(commandBuffer, descriptorSets);

		if (pushConstant)
		{
			vulkanPipeline->SetPushConstantData(commandBuffer, 0, pushConstant);
		}

		mesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		mesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), mesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DrawQuad()
	{
		LP_PROFILE_FUNCTION();
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);
		AllocateDescriptorsForQuadRendering();

		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->pipelineDescriptorSets);

		m_rendererStorage->quadMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DispatchRenderCommands(RenderBuffer& buffer)
	{
		LP_PROFILE_FUNCTION();
		if (m_rendererStorage->currentRenderPipeline->GetSpecification().drawSkybox && LevelManager::GetActive()->HasSkybox())
		{
			auto pipeline = m_rendererStorage->currentRenderPipeline;
			m_rendererStorage->currentRenderPipeline = LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetPipeline();
			LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->Draw();

			m_rendererStorage->currentRenderPipeline = pipeline;
		}

		if (m_rendererStorage->currentRenderPipeline->GetSpecification().drawTerrain && LevelManager::GetActive()->HasTerrain())
		{
			auto pipeline = m_rendererStorage->currentRenderPipeline;

			m_rendererStorage->currentRenderPipeline = LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->GetPipeline();
			LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->Draw();

			m_rendererStorage->currentRenderPipeline = pipeline;
		}

		switch (m_rendererStorage->currentRenderPipeline->GetSpecification().drawType)
		{
			case DrawType::Opaque:
			{
				for (const auto& command : buffer.drawCalls)
				{
					if (!command.material->GetMaterialData().useTranslucency && !command.material->GetMaterialData().useBlending)
					{
						DrawMesh(command.transform, command.data, command.material, command.id);
					}
				}
				break;
			}

			case DrawType::Translucency:
			{
				for (const auto& command : buffer.drawCalls)
				{
					if (command.material->GetMaterialData().useTranslucency)
					{
						DrawMesh(command.transform, command.data, command.material, command.id);
					}
				}
				break;
			}

			case DrawType::Transparent:
			{
				for (const auto& command : buffer.drawCalls)
				{
					if (command.material->GetMaterialData().useBlending)
					{
						DrawMesh(command.transform, command.data, command.material, command.id);	
					}
				}
				break;
			}

			case DrawType::Terrain:
			{
				LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->Draw();

				break;
			}

			case DrawType::Quad:
			{
				DrawQuad();

				break;
			}
		}
	}

	void Renderer::DispatchRenderCommands()
	{
		DispatchRenderCommands(m_finalRenderBuffer);
	}

	VkDescriptorSet Renderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		LP_PROFILE_FUNCTION();
		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
		
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];

		VkDescriptorSet descriptorSet;
		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet));

		return descriptorSet;
	}

	//TODO: Find better way to save execute function
	std::pair<Ref<RenderComputePipeline>, std::function<void()>> Renderer::CreateLightCullingPipeline(Ref<Image2D> depthImage)
	{
		Ref<Shader> lightCullingShader = ShaderLibrary::GetShader("lightCulling");
		Ref<VulkanRenderComputePipeline> lightCullingPipeline = std::reinterpret_pointer_cast<VulkanRenderComputePipeline>(RenderComputePipeline::Create(lightCullingShader));

		Ref<VulkanShader> vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(lightCullingShader);

		auto func = [=]()
		{
			LP_PROFILE_FUNCTION();

			auto device = VulkanContext::GetCurrentDevice();
			uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
			auto uniformBufferSet = m_rendererStorage->uniformBufferSet;

			auto descriptorLayout = vulkanShader->GetDescriptorSetLayout(0);
			auto& shaderDescriptorSet = vulkanShader->GetDescriptorSets()[0];

			//Allocate descriptors
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &descriptorLayout;

			VkDescriptorSet currentDescriptorSet;
			currentDescriptorSet = AllocateDescriptorSet(allocInfo);

			std::array<VkWriteDescriptorSet, 5> writeDescriptors;

			auto vulkanCameraDataBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(m_rendererStorage->uniformBufferSet->Get(0, 0, currentFrame));
			writeDescriptors[0] = *vulkanShader->GetDescriptorSet("CameraDataBuffer");
			writeDescriptors[0].dstSet = currentDescriptorSet;
			writeDescriptors[0].pBufferInfo = &vulkanCameraDataBuffer->GetDescriptorInfo();

			auto vulkanLightCullingBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(m_rendererStorage->lightCullingBuffer);
			writeDescriptors[1] = *vulkanShader->GetDescriptorSet("DirectionalLightBuffer");
			writeDescriptors[1].dstSet = currentDescriptorSet;
			writeDescriptors[1].pBufferInfo = &vulkanLightCullingBuffer->GetDescriptorInfo();

			auto vulkanLightStorageBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(m_rendererStorage->shaderStorageBufferSet->Get(12, 0, currentFrame));
			writeDescriptors[2] = *vulkanShader->GetDescriptorSet("LightBuffer");
			writeDescriptors[2].dstSet = currentDescriptorSet;
			writeDescriptors[2].pBufferInfo = &vulkanLightStorageBuffer->GetDescriptorInfo();

			auto vulkanVisibleLightsBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(m_rendererStorage->shaderStorageBufferSet->Get(13, 0, currentFrame));
			writeDescriptors[3] = *vulkanShader->GetDescriptorSet("VisibleLightsBuffer");
			writeDescriptors[3].dstSet = currentDescriptorSet;
			writeDescriptors[3].pBufferInfo = &vulkanVisibleLightsBuffer->GetDescriptorInfo();

			auto vulkanDepthImage = std::reinterpret_pointer_cast<VulkanImage2D>(depthImage);
			writeDescriptors[4] = *vulkanShader->GetDescriptorSet("u_DepthMap");
			writeDescriptors[4].dstSet = currentDescriptorSet;
			writeDescriptors[4].pImageInfo = &vulkanDepthImage->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
			lightCullingPipeline->Execute(&currentDescriptorSet, (uint32_t)1, m_rendererStorage->lightCullingRendererData.xTileCount, m_rendererStorage->lightCullingRendererData.yTileCount, 1);
		};

		return std::make_pair(lightCullingPipeline, func);
	}

	Renderer& Renderer::Get()
	{
		return *s_instance;
	}

	Ref<Renderer> Renderer::Create()
	{
		return CreateRef<Renderer>();
	}

	void Renderer::CreateUniformBuffers()
	{
		/////Uniform buffers//////
		m_rendererStorage->uniformBufferSet = UniformBufferSet::Create(s_capabilities.framesInFlight);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->cameraData, sizeof(CameraDataBuffer), 0, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->directionalLightDataBuffer, sizeof(DirectionalLightDataBuffer), 1, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->screenData, sizeof(ScreenDataBuffer), 3, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->directionalLightVPData, sizeof(DirectionalLightVPBuffer), 4, 0);

		m_rendererStorage->lightCullingBuffer = UniformBuffer::Create(&m_rendererStorage->lightCullingData, sizeof(LightCullingBuffer));
		m_rendererStorage->terrainDataBuffer = UniformBuffer::Create(&m_rendererStorage->terrainData, sizeof(TerrainDataBuffer));

		/////Shader storages/////
		auto& lightCullingData = m_rendererStorage->lightCullingRendererData;

		lightCullingData.xTileCount = (lightCullingData.maxScreenTileBufferAllocation + (lightCullingData.maxScreenTileBufferAllocation % lightCullingData.tileSize)) / lightCullingData.tileSize;
		lightCullingData.yTileCount = (lightCullingData.maxScreenTileBufferAllocation + (lightCullingData.maxScreenTileBufferAllocation % lightCullingData.tileSize)) / lightCullingData.tileSize;
		lightCullingData.tileCount = lightCullingData.xTileCount * lightCullingData.yTileCount;

		m_rendererStorage->shaderStorageBufferSet = ShaderStorageBufferSet::Create(s_capabilities.framesInFlight);
		m_rendererStorage->shaderStorageBufferSet->Add(lightCullingData.maxLights * sizeof(PointLightData), 12, 0);
		m_rendererStorage->shaderStorageBufferSet->Add(lightCullingData.tileCount * sizeof(LightIndex) * lightCullingData.maxLights, 13, 0);
	}

	void Renderer::UpdateUniformBuffers()
	{
		LP_PROFILE_FUNCTION();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Camera data
		{
			auto ub = m_rendererStorage->uniformBufferSet->Get(0, 0, currentFrame);

			Ref<PerspectiveCamera> perspectiveCamera = std::dynamic_pointer_cast<PerspectiveCamera>(m_rendererStorage->camera);
			float tanHalfFOV = glm::tan(glm::radians(perspectiveCamera->GetFieldOfView()) / 2.f);

			const bool levelLoaded = LevelManager::Get()->IsLevelLoaded();

			m_rendererStorage->cameraData.ambienceExposure.x = levelLoaded ? LevelManager::GetActive()->GetEnvironment().GetSkybox().ambianceMultiplier : 0.5f;
			m_rendererStorage->cameraData.ambienceExposure.y = levelLoaded ? LevelManager::GetActive()->GetEnvironment().GetSkybox().hdrExposure : 1.f;
			m_rendererStorage->cameraData.positionAndTanHalfFOV = glm::vec4(m_rendererStorage->camera->GetPosition(), tanHalfFOV);
			m_rendererStorage->cameraData.projection = m_rendererStorage->camera->GetProjectionMatrix();
			m_rendererStorage->cameraData.view = m_rendererStorage->camera->GetViewMatrix();

			ub->SetData(&m_rendererStorage->cameraData, sizeof(CameraDataBuffer));
		}

		//Directional lights
		if (LevelManager::IsLevelLoaded())
		{
			auto ub = m_rendererStorage->uniformBufferSet->Get(1, 0, currentFrame);

			uint32_t index = 0;
			m_rendererStorage->directionalLightDataBuffer.lightCount = 0;
			for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
			{
				glm::vec3 direction = glm::normalize(glm::mat3(light->transform) * glm::vec3(1.f));

				m_rendererStorage->directionalLightDataBuffer.dirLights[index].direction = glm::vec4(direction, 1.f);
				m_rendererStorage->directionalLightDataBuffer.dirLights[index].colorIntensity = glm::vec4(light->color, light->intensity);
				m_rendererStorage->directionalLightDataBuffer.dirLights[index].castShadows = light->castShadows;
				m_rendererStorage->directionalLightDataBuffer.lightCount++;

				index++;
				if (index > 0)
				{
					break;
				}
			}

			ub->SetData(&m_rendererStorage->directionalLightDataBuffer, sizeof(DirectionalLightDataBuffer));
		}

		//Terrain data
		{
			m_rendererStorage->terrainDataBuffer->SetData(&m_rendererStorage->terrainData, sizeof(TerrainDataBuffer));
		}


		//Light data
		if (LevelManager::IsLevelLoaded())
		{
			uint32_t index = 0;
			for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
			{
				m_rendererStorage->directionalLightVPData.directionalLightVPs[index] = light->viewProjection;
				index++;
			}
			m_rendererStorage->directionalLightVPData.count = index;

			auto ub = m_rendererStorage->uniformBufferSet->Get(4, 0, currentFrame);
			ub->SetData(&m_rendererStorage->directionalLightVPData, sizeof(DirectionalLightVPBuffer));
		}

		//Point lights
		if (LevelManager::IsLevelLoaded())
		{
			auto& pointLights = LevelManager::GetActive()->GetEnvironment().GetPointLights();
			auto pointlightStorageBuffer = m_rendererStorage->shaderStorageBufferSet->Get(12, 0, currentFrame);

			PointLightData* buffer = (PointLightData*)pointlightStorageBuffer->Map();

			m_rendererStorage->lightCullingData.lightCount = 0;
			m_rendererStorage->directionalLightDataBuffer.lightCount = 0;

			for (uint32_t i = 0; i < pointLights.size(); i++)
			{
				const auto& light = pointLights[i];

				buffer[i].position = glm::vec4(light->shadowBuffer->GetPosition(), 1.f);
				buffer[i].color = glm::vec4(light->color, 0.f);
				buffer[i].intensity = light->intensity;
				buffer[i].falloff = light->falloff;
				buffer[i].farPlane = light->farPlane;
				buffer[i].radius = light->radius;

				m_rendererStorage->lightCullingData.lightCount++;
				m_rendererStorage->directionalLightDataBuffer.pointLightCount++;
			}

			pointlightStorageBuffer->Unmap();

			m_rendererStorage->lightCullingBuffer->SetData(&m_rendererStorage->lightCullingData, sizeof(LightCullingBuffer));
		}
	}

	void Renderer::UpdatePerPassUniformBuffers(const Ref<RenderPipeline> pipeline)
	{
		LP_PROFILE_FUNCTION();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Light culling buffer
		{
			glm::vec2 size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };

			m_rendererStorage->lightCullingRendererData.xTileCount = ((uint32_t)size.x + (16 - ((uint32_t)size.x % 16))) / 16;
			m_rendererStorage->lightCullingRendererData.yTileCount = ((uint32_t)size.y + (16 - ((uint32_t)size.y % 16))) / 16;

			m_rendererStorage->lightCullingRendererData.tileCount = m_rendererStorage->lightCullingRendererData.xTileCount * m_rendererStorage->lightCullingRendererData.yTileCount;

			m_rendererStorage->lightCullingData.screenSize = size;
			m_rendererStorage->lightCullingBuffer->SetData(&m_rendererStorage->lightCullingData.screenSize, sizeof(glm::vec2));
		}

		//Screen
		{
			auto ub = m_rendererStorage->uniformBufferSet->Get(3, 0, currentFrame);

			m_rendererStorage->screenData.size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };
			m_rendererStorage->screenData.aspectRatio = m_rendererStorage->screenData.size.x / m_rendererStorage->screenData.size.y;
			m_rendererStorage->screenData.xScreenTiles = m_rendererStorage->lightCullingRendererData.xTileCount;

			ub->SetData(&m_rendererStorage->screenData, sizeof(ScreenDataBuffer));
		}
	}

	void Renderer::AllocateDescriptorsForMaterialRendering(Ref<Material> material)
	{
		LP_PROFILE_FUNCTION();

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		m_rendererStorage->currentMeshDescriptorSets.clear();
		m_rendererStorage->currentMeshDescriptorSets.emplace_back(m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET]);

		//Make sure that the shader has a descriptor set number 1
		if (vulkanShader->GetDescriptorSetLayoutCount() < PER_MESH_DESCRIPTOR_SET + 1)
		{
			return;
		}

		auto descriptorLayout = vulkanShader->GetDescriptorSetLayout(PER_MESH_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = vulkanShader->GetDescriptorSets()[PER_MESH_DESCRIPTOR_SET];

		//Allocate descriptors
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorLayout;

		auto& currentDescriptorSet = m_rendererStorage->currentMeshDescriptorSets.emplace_back();

		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &currentDescriptorSet));

		m_rendererStorage->pipelineDescriptorSets.emplace_back(currentDescriptorSet);

		//Update
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto& textureSpecification = vulkanMaterial->GetTextureSpecification();
		for (const auto& spec : textureSpecification)
		{
			if (spec.set != 1) // TODO: should only material texture descriptors be in material?
			{
				continue;
			}

			Ref<VulkanTexture2D> vulkanTexture;

			if (spec.texture)
			{
				vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(spec.texture);
			}
			else
			{
				vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(m_rendererDefaults->whiteTexture);
			}

			auto& imageSamplers = shaderDescriptorSet.imageSamplers;

			VulkanShader::ImageSampler* sampler = nullptr;

			auto imageNameIt = imageSamplers.find(spec.binding);
			if (imageNameIt != imageSamplers.end())
			{
				sampler = const_cast<VulkanShader::ImageSampler*>(&imageNameIt->second);
			}

			if (sampler)
			{
				auto descriptorWrite = shaderDescriptorSet.writeDescriptorSets.at(sampler->name);
				descriptorWrite.dstSet = currentDescriptorSet;
				descriptorWrite.pImageInfo = &vulkanTexture->GetDescriptorInfo();

				writeDescriptors.emplace_back(descriptorWrite);
			}
		}

		vkUpdateDescriptorSets(device->GetHandle(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
	}

	void Renderer::AllocateDescriptorsForQuadRendering()
	{
		LP_PROFILE_FUNCTION();

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();
		auto device = VulkanContext::GetCurrentDevice();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		/////Allocate//////
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];
		allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		allocInfo.pSetLayouts = allDescriptorLayouts.data();

		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets;
		currentDescriptorSet.resize(allDescriptorLayouts.size());

		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, currentDescriptorSet.data()));

		/////Update/////
		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();

		if (!shaderDescriptorSets.empty())
		{
			//Uniform buffers
			for (uint32_t set = 0; set < shaderDescriptorSets.size(); set++)
			{
				auto& uniformBuffers = shaderDescriptorSets[set].uniformBuffers;

				for (const auto& uniformBuffer : uniformBuffers)
				{
					auto writeDescriptor = shaderDescriptorSets[set].writeDescriptorSets.at(uniformBuffer.second->name);
					writeDescriptor.dstSet = currentDescriptorSet[set];

					auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(vulkanPipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, set, currentFrame));
					writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

					vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
				}
			}

			//Framebuffer textures
			auto& framebufferInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().framebufferInputs;
			for (const auto& framebufferInput : framebufferInputs)
			{
				if (framebufferInput.attachment)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(framebufferInput.attachment);
					auto& imageSamplers = shaderDescriptorSets[framebufferInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(framebufferInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[framebufferInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[framebufferInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{ 
					LP_CORE_ERROR("VulkanRenderer: No attachment bound to binding {0} in pipeline {1}!", framebufferInput.binding, "");
				}
			}
 		}
	}

	void Renderer::AllocatePerPassDescriptors()		
	{
		LP_PROFILE_FUNCTION();

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		LP_ASSERT(vulkanShader->GetDescriptorSetLayoutCount() > 0, "Descriptor set layouts must be greater than zero!");
		LP_ASSERT(!vulkanShader->GetDescriptorSets().empty(), "Descriptor set count must be greater than zero!");

		auto descriptorLayout = vulkanShader->GetDescriptorSetLayout(PER_PASS_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = vulkanShader->GetDescriptorSets()[PER_PASS_DESCRIPTOR_SET];

		//Allocate descriptors
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorLayout;

		m_rendererStorage->pipelineDescriptorSets.clear();
		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets.emplace_back();
		currentDescriptorSet = AllocateDescriptorSet(allocInfo);

		//Update descriptors
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto& uniformBuffers = shaderDescriptorSet.uniformBuffers;

		//Uniform buffers
		for (const auto& uniformBuffer : uniformBuffers)
		{
			auto writeDescriptor = shaderDescriptorSet.writeDescriptorSets.at(uniformBuffer.second->name);
			writeDescriptor.dstSet = currentDescriptorSet;

			auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(vulkanPipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, 0, currentFrame));
			writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

			writeDescriptors.emplace_back(writeDescriptor);
		}

		auto& shaderBuffers = shaderDescriptorSet.storageBuffers;
		//Shader buffers
		for (const auto& shaderBuffer : shaderBuffers)
		{
			auto writeDescriptor = shaderDescriptorSet.writeDescriptorSets.at(shaderBuffer.second->name);
			writeDescriptor.dstSet = currentDescriptorSet;

			auto vulkanShaderBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(vulkanPipeline->GetSpecification().shaderStorageBufferSets->Get(shaderBuffer.second->bindPoint, 0, currentFrame));
			writeDescriptor.pBufferInfo = &vulkanShaderBuffer->GetDescriptorInfo();

			writeDescriptors.emplace_back(writeDescriptor);
		}

		std::vector<VkDescriptorImageInfo> descriptorInfos;
		descriptorInfos.reserve(10);

		//Directional lights
		if (LevelManager::GetActive())
		{
			auto it = shaderDescriptorSet.writeDescriptorSets.find("u_DirShadowMaps");
			if (it != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = it->second;

				writeDescriptor.dstSet = currentDescriptorSet;

				uint32_t i = 0;
				for (auto light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
				{
					if (!light->castShadows)
					{
						continue;
					}

					if (i >= MAX_DIRECTIONAL_LIGHT_SHADOWS)
					{
						break;
					}

					descriptorInfos.emplace_back(std::reinterpret_pointer_cast<VulkanImage2D>(light->shadowBuffer->GetDepthAttachment())->GetDescriptorInfo());

					i++;
				}

				writeDescriptor.descriptorCount = descriptorInfos.size();
				writeDescriptor.pImageInfo = descriptorInfos.data();

				if (writeDescriptor.descriptorCount > 0)
				{
					writeDescriptors.emplace_back(writeDescriptor);
				}
			}
		}

		//Framebuffer textures
		auto& framebufferInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().framebufferInputs;
		for (const auto& framebufferInput : framebufferInputs)
		{
			Ref<VulkanImage2D> vulkanImage;

			if (framebufferInput.attachment)
			{
				vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(framebufferInput.attachment);
			}
			else
			{
				vulkanImage = std::reinterpret_pointer_cast<VulkanTexture2D>(m_rendererDefaults->whiteTexture)->GetImage();
			}

			auto& imageSamplers = shaderDescriptorSet.imageSamplers;

			auto imageSampler = imageSamplers.find(framebufferInput.binding);
			if (imageSampler != imageSamplers.end())
			{
				auto descriptorWrite = shaderDescriptorSet.writeDescriptorSets.at(imageSampler->second.name);
				descriptorWrite.dstSet = currentDescriptorSet;
				descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

				writeDescriptors.emplace_back(descriptorWrite);
			}
		}

		if (LevelManager::GetActive())
		{
			auto irradiance = shaderDescriptorSet.writeDescriptorSets.find("u_IrradianceMap");
			if (irradiance != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = irradiance->second;
				writeDescriptor.dstSet = currentDescriptorSet;
				writeDescriptor.descriptorCount = 1;

				if (LevelManager::GetActive()->HasSkybox())
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetIrradiance())->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(m_rendererDefaults->blackCubeTexture)->GetDescriptorInfo();
				}

				writeDescriptors.emplace_back(writeDescriptor);
			}

			auto prefilter = shaderDescriptorSet.writeDescriptorSets.find("u_PrefilterMap");
			if (prefilter != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = prefilter->second;
				writeDescriptor.dstSet = currentDescriptorSet;
				writeDescriptor.descriptorCount = 1;

				if (LevelManager::GetActive()->HasSkybox())
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetFilteredEnvironment())->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(m_rendererDefaults->blackCubeTexture)->GetDescriptorInfo();
				}

				writeDescriptors.emplace_back(writeDescriptor);
			}
		}

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}

	void Renderer::PreAllocateRenderBuffers()
	{
		m_firstRenderBuffer.drawCalls.reserve(500);
		m_secondRenderBuffer.drawCalls.reserve(500);

		m_finalRenderBuffer.drawCalls.reserve(500);
	}

	void Renderer::AllocateDescriptorPools()
	{
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

		m_descriptorPools.resize(s_capabilities.framesInFlight);
		auto device = VulkanContext::GetCurrentDevice();

		for (uint32_t i = 0; i < m_descriptorPools.size(); ++i)
		{
			LP_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPools[i]));
		}
	}

	void Renderer::DestroyDescriptorPools()
	{
		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());

		for (uint32_t i = 0; i < m_descriptorPools.size(); ++i)
		{
			vkDestroyDescriptorPool(device->GetHandle(), m_descriptorPools[i], nullptr);
		}

		m_descriptorPools.clear();
	}

	void Renderer::CreateRendererStorage()
	{
		m_rendererStorage = CreateScope<VulkanRendererStorage>();
		m_rendererDefaults = CreateScope<RendererDefaults>();

		//Setup default textures
		uint32_t blackCubeTextureData[6] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		m_rendererDefaults->blackCubeTexture = TextureCube::Create(ImageFormat::RGBA, 1, 1, &blackCubeTextureData);

		uint32_t whiteTextureData = 0xffffffff;
		m_rendererDefaults->whiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1);
		m_rendererDefaults->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		CreateUniformBuffers();
	}

	void Renderer::DestroyRendererStorage()
	{
		m_rendererDefaults.reset();
		m_rendererStorage.reset();
	}

	void Renderer::GenerateBRDF(Ref<Framebuffer>& outFramebuffer)
	{
		ScopedTimer timer{ "Generate BRDFLUT" };

		const uint32_t brdfDim = 512;

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.width = brdfDim;
		framebufferSpec.height = brdfDim;
		framebufferSpec.attachments =
		{
			ImageFormat::RG16F
		};

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		outFramebuffer = pipelineSpec.framebuffer;

		pipelineSpec.shader = ShaderLibrary::GetShader("BRDFIntegrate");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::Front;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Quad;
		pipelineSpec.uniformBufferSets = m_rendererStorage->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		auto renderPass = RenderPipeline::Create(pipelineSpec);

		m_rendererStorage->renderCommandBuffer->Begin();
		BeginPass(renderPass);
		DrawQuad();
		EndPass();
		m_rendererStorage->renderCommandBuffer->End();
	}

	void Renderer::PrepareForRender()
	{
		LP_PROFILE_FUNCTION();

		UpdateUniformBuffers();
		DrawDirectionalShadow();

		FrustumCull();
		SortRenderBuffer(m_rendererStorage->camera->GetPosition(), m_finalRenderBuffer);
	}

	void Renderer::SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& buffer)
	{
		std::sort(buffer.drawCalls.begin(), buffer.drawCalls.end(), [&sortPoint](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distTwo = glm::pow(sortPoint.x - dPosTwo.x, 2.f) + glm::pow(sortPoint.y - dPosTwo.y, 2.f) + glm::pow(sortPoint.z - dPosTwo.z, 2.f);
				const float distOne = glm::pow(sortPoint.x - dPosOne.x, 2.f) + glm::pow(sortPoint.y - dPosOne.y, 2.f) + glm::pow(sortPoint.z - dPosOne.z, 2.f);

				return distOne < distTwo;
			});
	}

	void Renderer::DrawDirectionalShadow()
	{
		LP_PROFILE_FUNCTION();

		if (!LevelManager::IsLevelLoaded())
		{
			return;
		}

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
		{
			if (!light->castShadows)
			{
				continue;
			}

			auto ub = light->shadowPipeline->GetSpecification().uniformBufferSets->Get(0, 0, currentFrame);
			ub->SetData(&light->viewProjection, sizeof(glm::mat4));

			BeginPass(light->shadowPipeline);

			DispatchRenderCommands(*m_renderBufferPointer);

			EndPass();
		}
	}

	void Renderer::FrustumCull()
	{
		auto frustum = m_rendererStorage->camera->CreateFrustum();

		for (auto& cmd : m_renderBufferPointer->drawCalls)
		{
			if (cmd.data->GetBoundingVolume().IsInFrustum(frustum, cmd.transform))
			{
				m_finalRenderBuffer.drawCalls.emplace_back(cmd);
			}
		}

		m_statistics.totalDrawCalls = m_finalRenderBuffer.drawCalls.size();
		m_statistics.culledDrawCalls = m_renderBufferPointer->drawCalls.size() - m_finalRenderBuffer.drawCalls.size();
	}

	void Renderer::ClearRenderBuffers()
	{
		m_firstRenderBuffer.drawCalls.clear();
		m_secondRenderBuffer.drawCalls.clear();
		
		m_finalRenderBuffer.drawCalls.clear();
	}
}