#pragma once

#include "Lamp/Core/Core.h"


namespace Lamp
{
	enum class DrawAccess
	{
		Static,
		Dynamic
	};

	class ShaderStorageBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() {}
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

		static Ref<ShaderStorageBuffer> Create(uint32_t size, uint32_t binding, DrawAccess access = DrawAccess::Static);
	};
}