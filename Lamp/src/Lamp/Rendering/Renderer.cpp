#include "lppch.h"
#include "Renderer.h"

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"
#include "Lamp/Rendering/LightBase.h"
#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"
#include "Lamp/Rendering/Textures/TextureCube.h"
#include "Lamp/Rendering/RenderPipelineLibrary.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Mesh/Materials/MaterialInstance.h"

#include "Lamp/AssetSystem/MeshImporter.h"
#include "Lamp/Level/Level.h"
#include "Lamp/Core/Time/ScopedTimer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include "Lamp/World/Terrain.h"
#include "Lamp/Utility/Random.h"

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
		GenerateBRDF(m_rendererDefaults->brdfFramebuffer);

		m_renderPipelineLibrary = CreateScope<RenderPipelineLibrary>();
		m_materialLibrary = CreateScope<MaterialLibrary>();

		MaterialLibrary::Get().LoadMaterials();
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
		auto swapchain = Application::Get().GetWindow().GetSwapchain();
		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		{
			LP_PROFILE_SCOPE("Reset descriptor pool");
			vkResetDescriptorPool(VulkanContext::GetCurrentDevice()->GetHandle(), m_descriptorPools[currentFrame], 0);
		}

		{
			LP_PROFILE_SCOPE("Command buffer begin");
			auto commandBuffer = m_rendererStorage->renderCommandBuffer;
			commandBuffer->Begin();
		}


		m_rendererStorage->camera = camera;
		m_statistics.memoryStatistics = GetMemoryUsage();

		if (camera)
		{
			LP_PROFILE_SCOPE("Prepare for render");
			PrepareForRender();
		}
	}

	void Renderer::End()
	{
		LP_PROFILE_FUNCTION();

		{
			LP_PROFILE_SCOPE("Command buffer submit");
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

		auto swapchain = Application::Get().GetWindow().GetSwapchain();
		auto framebuffer = pipeline->GetSpecification().framebuffer;
		auto commandBuffer = m_rendererStorage->renderCommandBuffer;

		framebuffer->Bind(commandBuffer);

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { 0, 0, framebuffer->GetSpecification().width, framebuffer->GetSpecification().height, };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = framebuffer->GetColorAttachmentInfos().size();
		renderingInfo.pColorAttachments = framebuffer->GetColorAttachmentInfos().data();

		if (framebuffer->GetDepthAttachment())
		{
			renderingInfo.pDepthAttachment = &framebuffer->GetDepthAttachmentInfo();
		}
		else
		{
			renderingInfo.pDepthAttachment = nullptr;
			renderingInfo.pStencilAttachment = nullptr;
		}

		vkCmdBeginRendering(commandBuffer->GetCurrentCommandBuffer(), &renderingInfo);

		pipeline->Bind(commandBuffer);
		
		AllocatePerPassDescriptors();
		pipeline->BindDescriptorSet(commandBuffer, m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET], PER_PASS_DESCRIPTOR_SET);
	}

	void Renderer::EndPass()
	{
		LP_PROFILE_FUNCTION();
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		vkCmdEndRendering(commandBuffer->GetCurrentCommandBuffer());
		m_rendererStorage->currentRenderPipeline->GetSpecification().framebuffer->Unbind(commandBuffer);
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

	const RendererStorage& Renderer::GetStorage() const
	{
		return *m_rendererStorage;
	}

	const RendererDefaults& Renderer::GetDefaults() const
	{
		return *m_rendererDefaults;
	}

	void Renderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, size_t id /* = -1 */)
	{
		LP_PROFILE_FUNCTION();
		m_submitBufferPointer->drawCalls.emplace_back(transform, mesh, material, id);
	}

	void Renderer::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant /* = nullptr */)
	{
		LP_PROFILE_FUNCTION();
		DrawMesh(mesh, material, descriptorSets, pushConstant);
	}

	void Renderer::DrawMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, size_t id)
	{
		LP_PROFILE_FUNCTION();
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		material->Bind(commandBuffer, nullptr);

		const auto& matData = material->GetSharedMaterial()->GetMaterialData();

		MeshData meshData;
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

		material->GetSharedMaterial()->GetPipeline()->SetPushConstantData(commandBuffer, 0, &transform);
		material->GetSharedMaterial()->GetPipeline()->SetPushConstantData(commandBuffer, 1, &meshData);

		mesh->GetVertexBuffer()->Bind(commandBuffer);
		mesh->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DrawMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		LP_PROFILE_FUNCTION();
		auto pipeline = m_rendererStorage->currentRenderPipeline;
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		pipeline->Bind(commandBuffer);
		pipeline->BindDescriptorSets(commandBuffer, descriptorSets);

		if (pushConstant)
		{
			pipeline->SetPushConstantData(commandBuffer, 0, pushConstant);
		}

		mesh->GetVertexBuffer()->Bind(commandBuffer);
		mesh->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DrawQuad()
	{
		LP_PROFILE_FUNCTION();
		auto pipeline = m_rendererStorage->currentRenderPipeline;
		auto commandBuffer = pipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		pipeline->Bind(commandBuffer);
		AllocateDescriptorsForQuadRendering();

		pipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->currentMeshDescriptorSets);

		m_rendererStorage->quadMesh->GetVertexBuffer()->Bind(commandBuffer);
		m_rendererStorage->quadMesh->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), m_rendererStorage->quadMesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::DispatchRenderCommands(RenderBuffer& buffer)
	{
		LP_PROFILE_FUNCTION();
		switch (m_rendererStorage->currentRenderPipeline->GetSpecification().drawType)
		{
			case DrawType::Opaque:
			{
				for (const auto& command : buffer.drawCalls)
				{
					if (!command.material->GetSharedMaterial()->GetMaterialData().useTranslucency && !command.material->GetSharedMaterial()->GetMaterialData().useBlending)
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
					if (command.material->GetSharedMaterial()->GetMaterialData().useTranslucency)
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
					if (command.material->GetSharedMaterial()->GetMaterialData().useBlending)
					{
						DrawMesh(command.transform, command.data, command.material, command.id);
					}
				}
				break;
			}

			case DrawType::Terrain:
			{
				if (LevelManager::GetActive()->HasTerrain())
				{
					LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->Draw(m_rendererStorage->currentRenderPipeline);
				}

				break;
			}

			case DrawType::Skybox:
			{
				if (LevelManager::GetActive()->HasSkybox())
				{
					LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->Draw(m_rendererStorage->currentRenderPipeline);
				}
				break;
			}

			case DrawType::FullscreenQuad:
			{
				DrawQuad();

				break;
			}

			case DrawType::Quad2D:
			{
				Renderer2D::Get().DispatchRenderCommands(DrawType::Quad2D);
				break;
			}

			case DrawType::Line2D:
			{
				Renderer2D::Get().DispatchRenderCommands(DrawType::Line2D);
				break;
			}
		}
	}

	void Renderer::DispatchRenderCommands()
	{
		auto currentRenderPipeline = m_rendererStorage->currentRenderPipeline->GetSpecification().pipelineType;

		DispatchRenderCommands(m_renderBufferMap[currentRenderPipeline]);
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
		Ref<RenderComputePipeline> lightCullingPipeline = RenderComputePipeline::Create(lightCullingShader);

		auto func = [=]() mutable
		{
			LP_PROFILE_SCOPE("LightCulling");

			auto device = VulkanContext::GetCurrentDevice();
			uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
			auto uniformBufferSet = m_rendererStorage->uniformBufferSet;

			auto descriptorLayout = lightCullingShader->GetDescriptorSetLayout(0);
			auto& shaderDescriptorSet = lightCullingShader->GetDescriptorSets()[0];

			//Allocate descriptors
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &descriptorLayout;

			VkDescriptorSet currentDescriptorSet;
			currentDescriptorSet = AllocateDescriptorSet(allocInfo);

			std::array<VkWriteDescriptorSet, 5> writeDescriptors;

			auto vulkanCameraDataBuffer = m_rendererStorage->uniformBufferSet->Get(0, 0, currentFrame);
			writeDescriptors[0] = *lightCullingShader->GetDescriptorSet("CameraDataBuffer");
			writeDescriptors[0].dstSet = currentDescriptorSet;
			writeDescriptors[0].pBufferInfo = &vulkanCameraDataBuffer->GetDescriptorInfo();

			auto vulkanLightCullingBuffer = m_rendererStorage->lightCullingBuffer;
			writeDescriptors[1] = *lightCullingShader->GetDescriptorSet("DirectionalLightBuffer");
			writeDescriptors[1].dstSet = currentDescriptorSet;
			writeDescriptors[1].pBufferInfo = &vulkanLightCullingBuffer->GetDescriptorInfo();

			auto vulkanLightStorageBuffer = m_rendererStorage->shaderStorageBufferSet->Get(12, 0, currentFrame);
			writeDescriptors[2] = *lightCullingShader->GetDescriptorSet("LightBuffer");
			writeDescriptors[2].dstSet = currentDescriptorSet;
			writeDescriptors[2].pBufferInfo = &vulkanLightStorageBuffer->GetDescriptorInfo();

			auto vulkanVisibleLightsBuffer = m_rendererStorage->shaderStorageBufferSet->Get(13, 0, currentFrame);
			writeDescriptors[3] = *lightCullingShader->GetDescriptorSet("VisibleLightsBuffer");
			writeDescriptors[3].dstSet = currentDescriptorSet;
			writeDescriptors[3].pBufferInfo = &vulkanVisibleLightsBuffer->GetDescriptorInfo();

			writeDescriptors[4] = *lightCullingShader->GetDescriptorSet("u_DepthMap");
			writeDescriptors[4].dstSet = currentDescriptorSet;
			writeDescriptors[4].pImageInfo = &depthImage->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);

			lightCullingPipeline->Begin(m_rendererStorage->renderCommandBuffer);
			lightCullingPipeline->Dispatch(currentDescriptorSet, m_rendererStorage->lightCullingRendererData.xTileCount, m_rendererStorage->lightCullingRendererData.yTileCount, 1);

			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(m_rendererStorage->renderCommandBuffer->GetCurrentCommandBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

			lightCullingPipeline->End();
		};

		return std::make_pair(lightCullingPipeline, func);
	}

	std::pair<Ref<RenderComputePipeline>, std::function<void()>> Renderer::CreateSSSBlurPass(Ref<Image2D> depthImage, Ref<Image2D> diffuseImage, Ref<Image2D> output)
	{
		Ref<Shader> sssBlurShader = ShaderLibrary::GetShader("sssBlur");
		Ref<RenderComputePipeline> sssBlurPipeline = RenderComputePipeline::Create(sssBlurShader);

		auto func = [=]() mutable
		{
			LP_PROFILE_SCOPE("SSSBlur");

			auto device = VulkanContext::GetCurrentDevice();
			auto descriptorSet = sssBlurShader->CreateDescriptorSets(0);

			uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

			std::array<VkWriteDescriptorSet, 5> writeDescriptors;
		};

		return std::pair<Ref<RenderComputePipeline>, std::function<void()>>();
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
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->cameraData, sizeof(CameraData), 0, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->directionalLightDataBuffer, sizeof(DirectionalLightDataData), 1, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->ssdoData, sizeof(SSDOData), 2, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->screenData, sizeof(ScreenData), 3, 0);
		m_rendererStorage->uniformBufferSet->Add(&m_rendererStorage->directionalLightVPData, sizeof(DirectionalLightVPData), 4, 0);

		m_rendererStorage->lightCullingBuffer = UniformBuffer::Create(&m_rendererStorage->lightCullingData, sizeof(LightCullingData));
		m_rendererStorage->terrainDataBuffer = UniformBuffer::Create(&m_rendererStorage->terrainData, sizeof(TerrainRenderData));

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

		Ref<PerspectiveCamera> perspectiveCamera = std::reinterpret_pointer_cast<PerspectiveCamera>(m_rendererStorage->camera);
		auto frustumPlanes = perspectiveCamera->CreateImplicitFrustum();


		//Camera data
		{
			auto ub = m_rendererStorage->uniformBufferSet->Get(0, 0, currentFrame);

			Ref<PerspectiveCamera> perspectiveCamera = std::reinterpret_pointer_cast<PerspectiveCamera>(m_rendererStorage->camera);
			float tanHalfFOV = glm::tan(glm::radians(perspectiveCamera->GetFieldOfView()) / 2.f);

			const bool levelLoaded = LevelManager::Get()->IsLevelLoaded();

			m_rendererStorage->cameraData.ambienceExposure.x = levelLoaded ? LevelManager::GetActive()->GetEnvironment().GetSkybox().ambianceMultiplier : 0.5f;
			m_rendererStorage->cameraData.ambienceExposure.y = levelLoaded ? LevelManager::GetActive()->GetEnvironment().GetSkybox().hdrExposure : 1.f;
			m_rendererStorage->cameraData.positionAndTanHalfFOV = glm::vec4(m_rendererStorage->camera->GetPosition(), tanHalfFOV);
			m_rendererStorage->cameraData.projection = m_rendererStorage->camera->GetProjectionMatrix();
			m_rendererStorage->cameraData.view = m_rendererStorage->camera->GetViewMatrix();

			ub->SetData(&m_rendererStorage->cameraData, sizeof(CameraData));
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

			ub->SetData(&m_rendererStorage->directionalLightDataBuffer, sizeof(DirectionalLightDataData));
		}

		//Terrain data
		{

			memcpy(m_rendererStorage->terrainData.frustumPlanes, frustumPlanes.data(), sizeof(glm::vec4) * frustumPlanes.size());
			m_rendererStorage->terrainDataBuffer->SetData(&m_rendererStorage->terrainData, sizeof(TerrainRenderData));
		}

		//SSDO
		{
			auto ub = m_rendererStorage->uniformBufferSet->Get(2, 0, currentFrame);
			ub->SetData(&m_rendererStorage->ssdoData, sizeof(SSDOData));
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
			ub->SetData(&m_rendererStorage->directionalLightVPData, sizeof(DirectionalLightVPData));
		}

		{
			m_rendererStorage->lightCullingBuffer->SetData(&m_rendererStorage->lightCullingData, sizeof(LightCullingData));
		}

		//Point lights
		if (LevelManager::IsLevelLoaded())
		{
			auto& pointLights = LevelManager::GetActive()->GetEnvironment().GetPointLights();
			auto pointlightStorageBuffer = m_rendererStorage->shaderStorageBufferSet->Get(12, 0, currentFrame);

			PointLightData* buffer = (PointLightData*)pointlightStorageBuffer->Map();

			m_rendererStorage->lightCullingData.lightCount = 0;
			m_rendererStorage->directionalLightDataBuffer.lightCount = 0;
			m_rendererStorage->directionalLightDataBuffer.pointLightCount = 0;

			for (uint32_t i = 0; i < pointLights.size(); i++)
			{
				const auto& light = pointLights[i];

				buffer[i].position = glm::vec4(light->position, 1.f);
				buffer[i].color = glm::vec4(light->color, 0.f);
				buffer[i].intensity = light->intensity;
				buffer[i].falloff = light->falloff;
				buffer[i].farPlane = light->farPlane;
				buffer[i].radius = light->radius;

				m_rendererStorage->lightCullingData.lightCount++;
				m_rendererStorage->directionalLightDataBuffer.pointLightCount++;
			}

			pointlightStorageBuffer->Unmap();
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

			ub->SetData(&m_rendererStorage->screenData, sizeof(ScreenData));
		}
	}

	void Renderer::DrawMeshWithPipeline(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, Ref<RenderPipeline> pipeline, size_t id)
	{
		LP_PROFILE_FUNCTION();
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		material->Bind(commandBuffer, pipeline);

		const auto& matData = material->GetSharedMaterial()->GetMaterialData();

		MeshData meshData;
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

		material->GetSharedMaterial()->GetPipeline()->SetPushConstantData(commandBuffer, 0, &transform);
		material->GetSharedMaterial()->GetPipeline()->SetPushConstantData(commandBuffer, 1, &meshData);

		mesh->GetVertexBuffer()->Bind(commandBuffer);
		mesh->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer->GetCurrentCommandBuffer(), mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void Renderer::AllocateDescriptorsForQuadRendering()
	{
		LP_PROFILE_FUNCTION();

		auto shader = m_rendererStorage->currentRenderPipeline->GetSpecification().shader;
		auto pipeline = m_rendererStorage->currentRenderPipeline;

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		m_rendererStorage->currentMeshDescriptorSets.clear();
		m_rendererStorage->currentMeshDescriptorSets.emplace_back(m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET]);

		//Make sure that the shader has a descriptor set number 1
		if (shader->GetDescriptorSetLayoutCount() < PER_MESH_DESCRIPTOR_SET + 1)
		{
			return;
		}

		auto descriptorLayout = shader->GetDescriptorSetLayout(PER_MESH_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = shader->GetDescriptorSets()[PER_MESH_DESCRIPTOR_SET];

		//Allocate descriptors
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorLayout;

		auto& currentDescriptorSet = m_rendererStorage->currentMeshDescriptorSets.emplace_back();

		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &currentDescriptorSet));

		m_rendererStorage->pipelineDescriptorSets.emplace_back(currentDescriptorSet);

		/////Update/////
		auto& shaderDescriptorSets = shader->GetDescriptorSets();

		if (!shaderDescriptorSets.empty())
		{
			//Uniform buffers
			auto& uniformBuffers = shaderDescriptorSets[PER_MESH_DESCRIPTOR_SET].uniformBuffers;

			for (const auto& uniformBuffer : uniformBuffers)
			{
				auto writeDescriptor = shaderDescriptorSets[PER_MESH_DESCRIPTOR_SET].writeDescriptorSets.at(uniformBuffer.second->name);
				writeDescriptor.dstSet = currentDescriptorSet;

				auto vulkanUniformBuffer = pipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, PER_MESH_DESCRIPTOR_SET, currentFrame);
				writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

				vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
			}

			//Framebuffer textures
			auto& framebufferInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().framebufferInputs;
			for (const auto& framebufferInput : framebufferInputs)
			{
				if (framebufferInput.set != PER_MESH_DESCRIPTOR_SET)
				{
					continue;
				}

				if (framebufferInput.attachment)
				{
					auto image = framebufferInput.attachment;
					auto& imageSamplers = shaderDescriptorSets[framebufferInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(framebufferInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[framebufferInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet;
						descriptorWrite.pImageInfo = &image->GetDescriptorInfo();

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

		if (m_rendererStorage->currentRenderPipeline->GetSpecification().drawType == DrawType::Quad2D)
		{
			return;
		}

		auto shader = m_rendererStorage->currentRenderPipeline->GetSpecification().shader;
		auto pipeline = m_rendererStorage->currentRenderPipeline;

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		LP_ASSERT(shader->GetDescriptorSetLayoutCount() > 0, "Descriptor set layouts must be greater than zero!");
		LP_ASSERT(!shader->GetDescriptorSets().empty(), "Descriptor set count must be greater than zero!");

		auto descriptorLayout = shader->GetDescriptorSetLayout(PER_PASS_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = shader->GetDescriptorSets()[PER_PASS_DESCRIPTOR_SET];

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

			auto vulkanUniformBuffer = pipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, 0, currentFrame);
			writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

			writeDescriptors.emplace_back(writeDescriptor);
		}

		auto& shaderBuffers = shaderDescriptorSet.storageBuffers;
		//Shader buffers
		for (const auto& shaderBuffer : shaderBuffers)
		{
			auto writeDescriptor = shaderDescriptorSet.writeDescriptorSets.at(shaderBuffer.second->name);
			writeDescriptor.dstSet = currentDescriptorSet;

			auto vulkanShaderBuffer = pipeline->GetSpecification().shaderStorageBufferSets->Get(shaderBuffer.second->bindPoint, 0, currentFrame);
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

					descriptorInfos.emplace_back(light->shadowBuffer->GetDepthAttachment()->GetDescriptorInfo());

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
			Ref<Image2D> vulkanImage;

			if (framebufferInput.attachment)
			{
				vulkanImage = framebufferInput.attachment;
			}
			else
			{
				vulkanImage = m_rendererDefaults->whiteTexture->GetImage();
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
					writeDescriptor.pImageInfo = &LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetIrradiance()->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &m_rendererDefaults->blackCubeTexture->GetDescriptorInfo();
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
					writeDescriptor.pImageInfo = &LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetFilteredEnvironment()->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &m_rendererDefaults->blackCubeTexture->GetDescriptorInfo();
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
		m_rendererStorage = CreateScope<RendererStorage>();
		m_rendererDefaults = CreateScope<RendererDefaults>();

		//Setup default textures
		uint32_t blackCubeTextureData[6] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		m_rendererDefaults->blackCubeTexture = TextureCube::Create(ImageFormat::RGBA, 1, 1, &blackCubeTextureData);

		uint32_t whiteTextureData = 0xffffffff;
		m_rendererDefaults->whiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1);
		m_rendererDefaults->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		m_rendererDefaults->ssdoNoise = Texture2D::Create(ImageFormat::RGBA32F, 4, 4);

		auto data = GenerateSSDONoise();
		m_rendererDefaults->ssdoNoise->SetData(data.data(), sizeof(glm::vec4) * data.size());

		GenerateSSDOKernel();
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
		pipelineSpec.drawType = DrawType::FullscreenQuad;
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
		DivideRenderBufferByPipeline(m_finalRenderBuffer);
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

	void Renderer::DivideRenderBufferByPipeline(RenderBuffer& renderBuffer)
	{
		LP_PROFILE_FUNCTION();

		m_renderBufferMap.clear();

		for (auto& draw : renderBuffer.drawCalls)
		{
			m_renderBufferMap[draw.material->GetSharedMaterial()->GetPipeline()->GetSpecification().pipelineType].drawCalls.emplace_back(draw);
		}
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

			light->shadowPipeline->Bind(m_rendererStorage->renderCommandBuffer);
			BeginPass(light->shadowPipeline);

			for (const auto& command : m_renderBufferPointer->drawCalls)
			{
				if (!command.material->GetSharedMaterial()->GetMaterialData().useBlending)
				{
					DrawMeshWithPipeline(command.transform, command.data, command.material, light->shadowPipeline, command.id);
				}
			}

			EndPass();
		}
	}

	static float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer::GenerateSSDOKernel()
	{
		for (uint32_t i = 0; i < m_rendererStorage->ssdoData.sizeBiasRadiusStrength.x; i++)
		{
			glm::vec3 sample{ Random::Float(0.f, 1.f) * 2.f - 1.f, Random::Float(0.f, 1.f) * 2.f - 1.f, Random::Float(0.f, 1.f) };
			sample = glm::normalize(sample);
			sample *= Random::Float(0.f, 1.f);

			float scale = (float)i / m_rendererStorage->ssdoData.sizeBiasRadiusStrength.x;

			scale = Lerp(0.1f, 1.f, scale * scale);
			sample *= scale;

			m_rendererStorage->ssdoData.kernelSamples[i] = glm::vec4(sample, 0.f);
		}
	}

	std::vector<glm::vec4> Renderer::GenerateSSDONoise()
	{
		std::vector<glm::vec4> noise;

		for (uint32_t i = 0; i < 16; i++)
		{
			noise.emplace_back(glm::vec4{ Random::Float(0.f, 1.f) * 2.f - 1.f, Random::Float(0.f, 1.f) * 2.f - 1.f, 0.f, 0.f });
		}

		return noise;
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