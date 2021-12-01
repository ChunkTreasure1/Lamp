#pragma once

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(const void* data, uint32_t size);
	};
}