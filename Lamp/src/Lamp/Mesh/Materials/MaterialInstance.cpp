#include "lppch.h"
#include "MaterialInstance.h"

#include "Lamp/Mesh/Materials/Material.h"

#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Platform/Vulkan/VulkanDevice.h"

namespace Lamp
{
	MaterialInstance::MaterialInstance(Ref<Material> sharedMaterial)
		: m_sharedMaterial(sharedMaterial)
	{}

	void MaterialInstance::Bind(Ref<CommandBuffer> commandBuffer, bool bindPipeline)
	{
		if (bindPipeline)
		{
			m_sharedMaterial->GetPipeline()->Bind(commandBuffer); //TODO: should only be called on first material. Probably in renderer?
		}

		std::vector<VkDescriptorSet> descriptors;
		AllocateDescriptorSets(descriptors);

		if (!descriptors.empty())
		{
			WriteToDescriptors(descriptors);
			m_sharedMaterial->GetPipeline()->BindDescriptorSets(commandBuffer, descriptors, 1);
		}
	}

	Ref<MaterialInstance> MaterialInstance::Create(Ref<Material> material)
	{
		return std::make_shared<MaterialInstance>(material);
	}

	void MaterialInstance::AllocateDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets)
	{
		auto shader = m_sharedMaterial->GetShader();

		if (shader->GetDescriptorSetLayoutCount() < 1)
		{
			return;
		}

		//Allocate descriptor sets for all sets above 0
		for (uint32_t i = 1; i < shader->GetDescriptorSetLayoutCount(); i++)
		{
			VkDescriptorSetLayout layout = shader->GetDescriptorSetLayout(i);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;

			outDescriptorSets.emplace_back(Renderer::Get().AllocateDescriptorSet(allocInfo)); // TODO: get rid of loop
		}
	}

	void MaterialInstance::WriteToDescriptors(const std::vector<VkDescriptorSet>& descriptorSets)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptors;
		
		auto pipeline = m_sharedMaterial->GetPipeline();
		auto shader = m_sharedMaterial->GetShader();

		auto& shaderDescriptorSets = shader->GetDescriptorSets();

		for (uint32_t i = 1; i < shaderDescriptorSets.size(); i++)
		{
			const auto& textureSpecification = m_sharedMaterial->GetTextureSpecification();
			for (const auto& spec : textureSpecification)
			{
				if (spec.set != i)
				{
					continue;
				}

				Ref<Texture2D> texture;
				if (spec.texture)
				{
					texture = spec.texture;
				}
				else
				{
					texture = Renderer::Get().GetDefaults().whiteTexture;
				}

				auto& imageSamplers = shaderDescriptorSets[i].imageSamplers;

				Shader::ImageSampler* sampler = nullptr;

				auto samplerIt = imageSamplers.find(spec.binding);
				if (samplerIt != imageSamplers.end())
				{
					sampler = const_cast<Shader::ImageSampler*>(&samplerIt->second);
				}

				if (sampler)
				{
					auto descriptorWrite = shaderDescriptorSets[i].writeDescriptorSets.at(sampler->name);
					descriptorWrite.dstSet = descriptorSets[i - 1];
					descriptorWrite.pImageInfo = &texture->GetDescriptorInfo();

					writeDescriptors.emplace_back(descriptorWrite);
				}
			}
		}

		auto device = VulkanContext::GetCurrentDevice();
		vkUpdateDescriptorSets(device->GetHandle(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}
}