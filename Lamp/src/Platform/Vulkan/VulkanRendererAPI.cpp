#include "lppch.h"
#include "VulkanRendererAPI.h"

namespace Lamp
{
	VulkanRendererAPI::VulkanRendererAPI()
	{
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
	}

	void VulkanRendererAPI::Initialize()
	{
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
	}

	void VulkanRendererAPI::Clear()
	{
	}

	void VulkanRendererAPI::ClearColor()
	{
	}

	void VulkanRendererAPI::ClearDepth()
	{
	}

	void VulkanRendererAPI::OffsetPolygon(float factor, float unit)
	{
	}

	void VulkanRendererAPI::SetCullFace(CullFace face) const
	{
	}

	void VulkanRendererAPI::EnableBlending(bool state) const
	{
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t)
	{
	}

	void VulkanRendererAPI::DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}
}