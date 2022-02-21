#include "lppch.h"
#include "Skybox.h"

#include "Lamp/Core/Time/ScopedTimer.h"

#include "Lamp/Rendering/Textures/TextureCube.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/RenderPipeline.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include "Lamp/Rendering/RendererDataStructures.h"
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/RenderCommand.h"

#include "Platform/Vulkan/VulkanRenderComputePipeline.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTextureCube.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		Path = path;
		if (!std::filesystem::exists(path))
		{
			LP_CORE_ERROR("[Skybox]: Unable to load file {0}!", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		Ref<Texture2D> hdrTexture = Texture2D::Create(path, false);

		const uint32_t cubemapSize = 1024;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
		Ref<TextureCube> envFiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
	
		Ref<Shader> conversionShader = ShaderLibrary::GetShader("equirectangularToCubeMap");
		Ref<VulkanRenderComputePipeline> equirectangularConversionPipeline = std::reinterpret_pointer_cast<VulkanRenderComputePipeline>(RenderComputePipeline::Create(conversionShader));

		auto device = VulkanContext::GetCurrentDevice();
	
		//Unfiltered
		{
			auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(conversionShader);

			std::array<VkWriteDescriptorSet, 2> writeDescriptors;
			auto descriptorSet = vulkanShader->CreateDescriptorSets();

			auto vulkanEnvUnfiltered = std::reinterpret_pointer_cast<VulkanTextureCube>(envUnfiltered);
			writeDescriptors[0] = *vulkanShader->GetDescriptorSet("o_CubeMap");
			writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[0].pImageInfo = &vulkanEnvUnfiltered->GetDescriptorInfo();

			auto vulkanEquiRect = std::reinterpret_pointer_cast<VulkanTexture2D>(hdrTexture);
			writeDescriptors[1] = *vulkanShader->GetDescriptorSet("u_EquirectangularTex");
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &vulkanEquiRect->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, NULL);
			equirectangularConversionPipeline->Execute(descriptorSet.descriptorSets.data(), (uint32_t)descriptorSet.descriptorSets.size(), cubemapSize / 32, cubemapSize / 32, 6);
			vulkanEnvUnfiltered->GenerateMips(true);
		}

		Ref<Shader> environmentMipFilterShader = ShaderLibrary::GetShader("environmentMipFilter");
		Ref<VulkanRenderComputePipeline> environmentMipFilterPipeline = std::reinterpret_pointer_cast<VulkanRenderComputePipeline>(RenderComputePipeline::Create(environmentMipFilterShader));

		//Mip filter
		{
			Ref<VulkanShader> vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(environmentMipFilterShader);
			Ref<VulkanTextureCube> envFilteredCubemap = std::reinterpret_pointer_cast<VulkanTextureCube>(envFiltered);
		
			VkDescriptorImageInfo imageInfo = envFilteredCubemap->GetDescriptorInfo();
			uint32_t mipCount = Utility::CalculateMipCount(cubemapSize, cubemapSize);

			std::vector<VkWriteDescriptorSet> writeDescriptors(mipCount * 2);
			std::vector<VkDescriptorImageInfo> mipImageInfos(mipCount);

			auto descriptorSet = vulkanShader->CreateDescriptorSets(0, 12);
			for (uint32_t i = 0; i < mipCount; i++)
			{
				VkDescriptorImageInfo& mipImageInfo = mipImageInfos[i];
				mipImageInfo = imageInfo;
				mipImageInfo.imageView = envFilteredCubemap->CreateImageViewSingleMip(i);

				writeDescriptors[i * 2 + 0] = *vulkanShader->GetDescriptorSet("outputTexture");
				writeDescriptors[i * 2 + 0].dstSet = descriptorSet.descriptorSets[i];
				writeDescriptors[i * 2 + 0].pImageInfo = &mipImageInfo;

				Ref<VulkanTextureCube> envUnfilteredCubemap = std::reinterpret_pointer_cast<VulkanTextureCube>(envUnfiltered);
				writeDescriptors[i * 2 + 1] = *vulkanShader->GetDescriptorSet("inputTexture");
				writeDescriptors[i * 2 + 1].dstSet = descriptorSet.descriptorSets[i];
				writeDescriptors[i * 2 + 1].pImageInfo = &envUnfilteredCubemap->GetDescriptorInfo();
			}

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);

			environmentMipFilterPipeline->Begin();
			const float deltaRoughness = 1.f / glm::max((float)envFiltered->GetMipLevelCount() - 1.f, 1.f);
			
			for (uint32_t i = 0, size = cubemapSize; i < mipCount; i++, size /= 2)
			{
				uint32_t numGroups = glm::max(1u, size / 32);
				
				float roughness = i * deltaRoughness;
				roughness = glm::max(roughness, 0.05f);

				environmentMipFilterPipeline->SetPushConstants(&roughness, sizeof(float));
				environmentMipFilterPipeline->Dispatch(descriptorSet.descriptorSets[i], numGroups, numGroups, 6);
			}

			environmentMipFilterPipeline->End();
		}

		Ref<Shader> irradianceShader = ShaderLibrary::GetShader("environmentIrradiance");
		Ref<VulkanRenderComputePipeline> environmentIrradiancePipeline = std::reinterpret_pointer_cast<VulkanRenderComputePipeline>(RenderComputePipeline::Create(irradianceShader));
		Ref<TextureCube> irradianceMap = TextureCube::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize);

		//Irradiance
		{
			Ref<VulkanShader> vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(irradianceShader);

			Ref<VulkanTextureCube> envFilteredCubemap = std::reinterpret_pointer_cast<VulkanTextureCube>(envFiltered);
			Ref<VulkanTextureCube> irradianceCubemap = std::reinterpret_pointer_cast<VulkanTextureCube>(irradianceMap);

			auto descriptorSet = vulkanShader->CreateDescriptorSets();

			std::array<VkWriteDescriptorSet, 2> writeDescriptors;
			writeDescriptors[0] = *vulkanShader->GetDescriptorSet("o_IrradianceMap");
			writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[0].pImageInfo = &irradianceCubemap->GetDescriptorInfo();

			writeDescriptors[1] = *vulkanShader->GetDescriptorSet("u_RadianceMap");
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &envFilteredCubemap->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		
			const uint32_t irradianceComputeSamples = 512;

			environmentIrradiancePipeline->Begin();
			environmentIrradiancePipeline->SetPushConstants(&irradianceComputeSamples, sizeof(uint32_t));
			environmentIrradiancePipeline->Dispatch(descriptorSet.descriptorSets[0], irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
			environmentIrradiancePipeline->End();

			irradianceCubemap->GenerateMips(false);
		}

		m_irradianceMap = irradianceMap;
		m_filteredEnvironment = envFiltered;

		m_cubeMesh = SubMesh::CreateCube();
	}

	Skybox::~Skybox()
	{
		if (m_descriptorSet.pool)
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkDestroyDescriptorPool(device->GetHandle(), m_descriptorSet.pool, nullptr);
			m_descriptorSet.pool = nullptr;
		}
	}

	void Skybox::Draw(Ref<RenderPipeline> pipeline)
	{
		struct SkyboxData
		{
			float environmentLod = 1.f;
			float environmentMultiplier = 1.f;

		} skyData;

		skyData.environmentLod = LevelManager::GetActive()->GetEnvironment().GetSkybox().environmentLod;
		skyData.environmentMultiplier = LevelManager::GetActive()->GetEnvironment().GetSkybox().environmentMultiplier;

		SetupDescriptors(pipeline);

		RenderCommand::SubmitMesh(m_cubeMesh, nullptr, m_descriptorSet.descriptorSets, static_cast<void*>(&skyData));
	}

	void Skybox::SetupDescriptors(Ref<RenderPipeline> pipeline)
	{
		if (m_descriptorSet.pool)
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkDestroyDescriptorPool(device->GetHandle(), m_descriptorSet.pool, nullptr);
			m_descriptorSet.pool = nullptr;
		}

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(pipeline->GetSpecification().shader);
		auto device = VulkanContext::GetCurrentDevice();

		auto descriptorSet = vulkanShader->CreateDescriptorSets();

		auto vulkanEnvironment = std::reinterpret_pointer_cast<VulkanTextureCube>(m_filteredEnvironment);
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(pipeline->GetSpecification().uniformBufferSets->Get(0, 0, currentFrame));

		writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("CameraDataBuffer"));
		writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
		writeDescriptors[0].pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

		if (vulkanEnvironment)
		{
			writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("u_EnvironmentMap"));
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &vulkanEnvironment->GetDescriptorInfo();
		}
		else
		{
			writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("u_EnvironmentMap"));
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(Renderer::Get().GetDefaults().blackCubeTexture)->GetDescriptorInfo();
		} 

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		m_descriptorSet = descriptorSet;
	}
}