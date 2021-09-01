#pragma once

#include <vector>
#include <string>

#include "Lamp/Utility/Types.h"

namespace Lamp
{

	struct BufferElement
	{
		BufferElement(Type elementType, const std::string& name, bool normalized = false)
			: Name(name), ElementType(elementType), Size(GetSizeFromType(elementType)), Offset(0), Normalized(normalized)
		{}

		uint32_t GetComponentCount(Type elementType)
		{
			switch (elementType)
			{
				case Lamp::Type::Bool: return 1;
				case Lamp::Type::Int: return 1;
				case Lamp::Type::Float: return 1;
				case Lamp::Type::Float2: return 2;
				case Lamp::Type::Float3: return 3;
				case Lamp::Type::Float4: return 4;
				case Lamp::Type::Mat3: return 3 * 3;
				case Lamp::Type::Mat4: return 4 * 4;
			}
		}

		bool Normalized = false;

		std::string Name;
		size_t Offset;
		uint32_t Size;
		Type ElementType;
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline std::vector<BufferElement>& GetElements() { return m_Elements; }

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
}