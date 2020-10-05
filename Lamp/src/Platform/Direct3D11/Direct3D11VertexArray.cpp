#include "lppch.h"
#include "Direct3D11VertexArray.h"

#include <d3d11.h>

namespace Lamp
{
	static DXGI_FORMAT ElementTypeToDXEnum(ElementType type)
	{
		switch (type)
		{
			case Lamp::ElementType::Bool: return DXGI_FORMAT_R8_SINT;
			case Lamp::ElementType::Int: return DXGI_FORMAT_R32_SINT;
			case Lamp::ElementType::Float: return DXGI_FORMAT_R32_FLOAT;
			case Lamp::ElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
			case Lamp::ElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
			case Lamp::ElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case Lamp::ElementType::Mat3: return DXGI_FORMAT_R32_FLOAT;
			case Lamp::ElementType::Mat4: return DXGI_FORMAT_R32_FLOAT;
		}
	}

	Direct3D11VertexArray::Direct3D11VertexArray()
	{
	}

	Direct3D11VertexArray::~Direct3D11VertexArray()
	{
	}

	void Direct3D11VertexArray::Bind()
	{
	}

	void Direct3D11VertexArray::Unbind()
	{
	}

	void Direct3D11VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer)
	{
	}

	void Direct3D11VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer)
	{
	}
}