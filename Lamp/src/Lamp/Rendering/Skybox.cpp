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
#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanUtility.h"

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
		Ref<RenderComputePipeline> equirectangularConversionPipeline = RenderComputePipeline::Create(conversionShader);

		auto device = VulkanContext::GetCurrentDevice();
	
		//Unfiltered
		{
			std::array<VkWriteDescriptorSet, 2> writeDescriptors;
			auto descriptorSet = conversionShader->CreateDescriptorSets();

			writeDescriptors[0] = *conversionShader->GetDescriptorSet("o_CubeMap");
			writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[0].pImageInfo = &envUnfiltered->GetDescriptorInfo();

			writeDescriptors[1] = *conversionShader->GetDescriptorSet("u_EquirectangularTex");
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &hdrTexture->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, NULL);
			equirectangularConversionPipeline->Execute(descriptorSet.descriptorSets.data(), (uint32_t)descriptorSet.descriptorSets.size(), cubemapSize / 32, cubemapSize / 32, 6);
			envUnfiltered->GenerateMips(true);
		}

		Ref<Shader> environmentMipFilterShader = ShaderLibrary::GetShader("environmentMipFilter");
		Ref<RenderComputePipeline> environmentMipFilterPipeline = RenderComputePipeline::Create(environmentMipFilterShader);

		//Mip filter
		{
			VkDescriptorImageInfo imageInfo = envFiltered->GetDescriptorInfo();
			uint32_t mipCount = Utility::CalculateMipCount(cubemapSize, cubemapSize);

			std::vector<VkWriteDescriptorSet> writeDescriptors(mipCount * 2);
			std::vector<VkDescriptorImageInfo> mipImageInfos(mipCount);

			auto descriptorSet = environmentMipFilterShader->CreateDescriptorSets(0, 12);
			for (uint32_t i = 0; i < mipCount; i++)
			{
				VkDescriptorImageInfo& mipImageInfo = mipImageInfos[i];
				mipImageInfo = imageInfo;
				mipImageInfo.imageView = envFiltered->CreateImageViewSingleMip(i);

				writeDescriptors[i * 2 + 0] = *environmentMipFilterShader->GetDescriptorSet("outputTexture");
				writeDescriptors[i * 2 + 0].dstSet = descriptorSet.descriptorSets[i];
				writeDescriptors[i * 2 + 0].pImageInfo = &mipImageInfo;

				writeDescriptors[i * 2 + 1] = *environmentMipFilterShader->GetDescriptorSet("inputTexture");
				writeDescriptors[i * 2 + 1].dstSet = descriptorSet.descriptorSets[i];
				writeDescriptors[i * 2 + 1].pImageInfo = &envUnfiltered->GetDescriptorInfo();
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
		Ref<RenderComputePipeline> environmentIrradiancePipeline = RenderComputePipeline::Create(irradianceShader);
		Ref<TextureCube> irradianceMap = TextureCube::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize);

		//Irradiance
		{
			auto descriptorSet = irradianceShader->CreateDescriptorSets();

			std::array<VkWriteDescriptorSet, 2> writeDescriptors;
			writeDescriptors[0] = *irradianceShader->GetDescriptorSet("o_IrradianceMap");
			writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[0].pImageInfo = &irradianceMap->GetDescriptorInfo();

			writeDescriptors[1] = *irradianceShader->GetDescriptorSet("u_RadianceMap");
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &envFiltered->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		
			const uint32_t irradianceComputeSamples = 512;

			environmentIrradiancePipeline->Begin();
			environmentIrradiancePipeline->SetPushConstants(&irradianceComputeSamples, sizeof(uint32_t));
			environmentIrradiancePipeline->Dispatch(descriptorSet.descriptorSets[0], irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
			environmentIrradiancePipeline->End();

			irradianceMap->GenerateMips(false);
		}

		m_irradianceMap = irradianceMap;
		m_filteredEnvironment = envFiltered;

		m_cubeMesh = SubMesh::CreateCube();
		
		m_pipeline = Renderer::Get().GetStorage().skyboxPipeline;
		SetupDescriptors();
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

	void Skybox::Draw()
	{
		struct SkyboxData
		{
			float environmentLod = 1.f;
			float environmentMultiplier = 1.f;

		} skyData;

		skyData.environmentLod = LevelManager::GetActive()->GetEnvironment().GetSkybox().environmentLod;
		skyData.environmentMultiplier = LevelManager::GetActive()->GetEnvironment().GetSkybox().environmentMultiplier;

		RenderCommand::SubmitMesh(m_cubeMesh, nullptr, m_descriptorSet.descriptorSets, static_cast<void*>(&skyData));
	}

	void Skybox::SetupDescriptors()
	{
		if (m_descriptorSet.pool)
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkDestroyDescriptorPool(device->GetHandle(), m_descriptorSet.pool, nullptr);
			m_descriptorSet.pool = nullptr;
		}

		auto shader = m_pipeline->GetSpecification().shader;
		auto device = VulkanContext::GetCurrentDevice();

		auto descriptorSet = shader->CreateDescriptorSets();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto uniformBuffer = m_pipeline->GetSpecification().uniformBufferSets->Get(0, 0, currentFrame);

		writeDescriptors.emplace_back(*shader->GetDescriptorSet("CameraDataBuffer"));
		writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
		writeDescriptors[0].pBufferInfo = &uniformBuffer->GetDescriptorInfo();

		if (m_filteredEnvironment)
		{
			writeDescriptors.emplace_back(*shader->GetDescriptorSet("u_EnvironmentMap"));
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pImageInfo = &m_filteredEnvironment->GetDescriptorInfo();
		}

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		m_descriptorSet = descriptorSet;
	}
}