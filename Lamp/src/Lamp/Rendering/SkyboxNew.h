#pragma once

#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class Framebuffer;
	class SubMesh;

	class SkyboxNew
	{
	public:
		SkyboxNew();
		~SkyboxNew();
	
		void GenerateBRDF();
		void GenerateIrradianceCube();
		void GeneratePrefilterCube();

	private:
		Ref<Framebuffer> m_brdfFramebuffer;
		Ref<SubMesh> m_cubeMesh;

		VmaAllocation m_irradianceAlloc;
		VkImage m_irradianceImage;
		VkImageView m_irradianceImageView;
		VkSampler m_irradianceSampler;
		VkDescriptorImageInfo m_irradianceDescriptor;

		VkDescriptorImageInfo m_cubeDescriptor;
	};
}