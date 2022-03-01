#pragma once

#include "Lamp/Core/Core.h"

#include <vulkan/vulkan.h>

namespace Lamp
{
	class Material;
	class CommandBuffer;
	class MaterialInstance
	{
	public:
		MaterialInstance(Ref<Material> sharedMaterial);
		~MaterialInstance() = default;

		void Bind(Ref<CommandBuffer> commandBuffer, Ref<RenderPipeline> pipeline);

		inline Ref<Material> GetSharedMaterial() const { return m_sharedMaterial; }
		static Ref<MaterialInstance> Create(Ref<Material> material);

	private:
		void AllocateDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets);
		void WriteToDescriptors(const std::vector<VkDescriptorSet>& descriptorSets);

		const Ref<Material> m_sharedMaterial;
	};
}