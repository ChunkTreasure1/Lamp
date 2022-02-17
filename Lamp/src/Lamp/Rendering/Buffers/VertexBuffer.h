#pragma once

#include "BufferLayout.h"

#include "Lamp/Rendering/Vertex.h"

#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <glm/glm.hpp>

#include <functional>

namespace Lamp
{
	class CommandBuffer;
	class VertexBuffer
	{
	public:
		VertexBuffer(const std::vector<Vertex>& vertices, uint32_t size);
		VertexBuffer(const std::vector<float>& vertices, uint32_t size);
		VertexBuffer(uint32_t size);

		~VertexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const;
		void Unbind() const;

		void SetData(const void* data, uint32_t size);

		inline VkBuffer GetBuffer() { return m_buffer; }

		static Ref<VertexBuffer> Create(const std::vector<Vertex>& pVertices, uint32_t size);
		static Ref<VertexBuffer> Create(const std::vector<float>& vertices, uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size);

	private:
		VkBuffer m_buffer = nullptr;
		VmaAllocation m_bufferAllocation = nullptr;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(const std::vector<uint32_t>& indices, uint32_t count);
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		void Bind(Ref<CommandBuffer> commandBuffer) const;
		void Unbind() const;

		uint32_t GetCount();
		inline VkBuffer GetBuffer() { return m_buffer; }

		static Ref<IndexBuffer> Create(const std::vector<uint32_t>& pIndices, uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* pIndices, uint32_t count);

	private:
		uint32_t m_count;
		VkBuffer m_buffer;
		VmaAllocation m_bufferAllocation;
	};
}