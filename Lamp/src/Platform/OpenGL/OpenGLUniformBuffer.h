#pragma once

#include "Lamp/Rendering/Buffers/UniformBuffer.h"

namespace Lamp
{
	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(const void* data, uint32_t size);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID = 0;
	};
}