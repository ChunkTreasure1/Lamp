#pragma once

#include "Lamp/Core/Core.h"
#include <functional>

namespace Lamp
{
	enum class ShaderDataType
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int,
		Int2,
		Int3,
		Int4,
		Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case Lamp::ShaderDataType::Float:	return 4;
			case Lamp::ShaderDataType::Float2:	return 4 * 2;
			case Lamp::ShaderDataType::Float3:	return 4 * 3;
			case Lamp::ShaderDataType::Float4:	return 4 * 4;
			case Lamp::ShaderDataType::Mat3:	return 4 * 3 * 3;
			case Lamp::ShaderDataType::Mat4:	return 4 * 4 * 4;
			case Lamp::ShaderDataType::Int:		return 4;
			case Lamp::ShaderDataType::Int2:	return 4 * 2;
			case Lamp::ShaderDataType::Int3:	return 4 * 3;
			case Lamp::ShaderDataType::Int4:	return 4 * 4;
			case Lamp::ShaderDataType::Bool:	return 1;
		}

		return 0;
	}

	struct BufferElement
	{
		std::string name;
		ShaderDataType Type;
		uint32_t size;
		size_t offset;
		bool normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), Type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
		{}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:   return 1;
				case ShaderDataType::Float2:  return 2;
				case ShaderDataType::Float3:  return 3;
				case ShaderDataType::Float4:  return 4;
				case ShaderDataType::Mat3:    return 3; // 3* float3
				case ShaderDataType::Mat4:    return 4; // 4* float4
				case ShaderDataType::Int:     return 1;
				case ShaderDataType::Int2:    return 2;
				case ShaderDataType::Int3:    return 3;
				case ShaderDataType::Int4:    return 4;
				case ShaderDataType::Bool:    return 1;
			}

			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_elements(elements)
		{
			CalculateOffsetAndStride();
		}

		uint32_t GetStride() { return m_stride; }
		const std::vector<BufferElement>& GetElements() const { return m_elements; }

		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			m_stride = 0;
			for (auto& element : m_elements)
			{
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
	};
}