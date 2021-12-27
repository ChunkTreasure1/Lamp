#include "lppch.h"
#include "Material.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanMaterial.h"

namespace Lamp
{
	Ref<Material> Material::Create(Ref<Shader> shader, uint32_t id)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMaterial>(shader, id);
		}

		return nullptr;
	}

	Ref<Material> Material::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMaterial>();
		}

		return nullptr;
	}

	Ref<Material> Material::Create(const Ref<Material> material)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMaterial>(material);
		}

		return nullptr;
	}

	Ref<Material> Material::Create(const std::string& name, uint32_t index)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMaterial>(name, index);
		}

		return nullptr;
	}
}