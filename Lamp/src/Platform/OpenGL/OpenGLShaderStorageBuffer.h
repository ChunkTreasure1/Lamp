#pragma once

#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"

namespace Lamp
{
	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLShaderStorageBuffer();

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void* Map() override;
		virtual void Unmap() override;

	private:
		uint32_t m_RendererID = 0;
	};
}