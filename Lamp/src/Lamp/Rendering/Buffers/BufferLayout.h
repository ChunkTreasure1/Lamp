#pragma once

#include <vector>
#include <string>

namespace Lamp
{
	enum class ElementType : uint32_t
	{
		Int = 0,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4
	};

	static uint32_t GetSizeFromType(ElementType elementType)
	{
		switch (elementType)
		{
			case ElementType::Bool: return 1;
			case ElementType::Int: return 4;
			case ElementType::Float: return 4;
			case ElementType::Float2: return 4 * 2;
			case ElementType::Float3: return 4 * 3;
			case ElementType::Float4: return 4 * 4;
			case ElementType::Mat3: return 4 * 3 * 3;
			case ElementType::Mat4: return 4 * 4 * 4;
		}

		return 0;
	}

	struct BufferElement
	{
		BufferElement(ElementType elementType, const std::string& name, bool normalized = false)
			: name(name), elementType(elementType), size(GetSizeFromType(elementType)), offset(0), normalized(normalized)
		{}

		uint32_t GetComponentCount(ElementType elementType)
		{
			switch (elementType)
			{
				case ElementType::Bool: return 1;
				case ElementType::Int: return 1;
				case ElementType::Float: return 1;
				case ElementType::Float2: return 2;
				case ElementType::Float3: return 3;
				case ElementType::Float4: return 4;
				case ElementType::Mat3: return 3 * 3;
				case ElementType::Mat4: return 4 * 4;
			}

			return 0;
		}


		std::string name;
		size_t offset;
		uint32_t size;
		ElementType type;

		bool normalized = false;
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_elements(elements)
		{
			CalculateOffsetAndStride();
		}

		inline uint32_t GetStride() const { return m_stride; }
		inline std::vector<BufferElement>& GetElements() { return m_elements; }

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

		friend class RenderNodePass;

		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
	};
}