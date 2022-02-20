#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"

#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t size);
		VulkanVertexBuffer(const std::vector<float>& vertices, uint32_t size);
		VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer() override;

		void Bind() const override;
		void Bind(Ref<CommandBuffer> commandBuffer) const override;
		void Unbind() const override;

		void SetBufferLayout(const BufferLayout& buff) override {}
		BufferLayout& GetBufferLayout() override { return BufferLayout(); }

		void SetVertices(std::vector<Vertex>& pVertices, uint32_t size) override;
		void SetData(const void* data, uint32_t size) override;

		//TODO: remove
		inline VkBuffer GetBuffer() { return m_buffer; }

	private:
		VkBuffer m_buffer = nullptr;
		VmaAllocation m_bufferAllocation = nullptr;
	};

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const std::vector<uint32_t>& indices, uint32_t count);
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer() override;

		void Bind() const override;
		void Bind(Ref<CommandBuffer> commandBuffer) const override;
		void Unbind() const override;

		uint32_t GetCount() override;

		//TODO: remove
		inline VkBuffer GetBuffer() { return m_buffer; }

	private:
		uint32_t m_count;

		VkBuffer m_buffer;
		VmaAllocation m_bufferAllocation;
	};
}