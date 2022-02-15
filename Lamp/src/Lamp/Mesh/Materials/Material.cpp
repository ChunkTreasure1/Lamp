#include "lppch.h"
#include "Material.h"

#include "Platform/Vulkan/VulkanMaterial.h"

namespace Lamp
{
	Ref<Material> Material::Create(Ref<Shader> shader, uint32_t id)
	{
		return CreateRef<VulkanMaterial>(shader, id);
	}

	Ref<Material> Material::Create()
	{
		return CreateRef<VulkanMaterial>();
	}

	Ref<Material> Material::Create(const Ref<Material> material)
	{
		return CreateRef<VulkanMaterial>(material);
	}

	Ref<Material> Material::Create(const std::string& name, uint32_t index)
	{
		return CreateRef<VulkanMaterial>(name, index);
	}
}