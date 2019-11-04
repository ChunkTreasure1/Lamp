#include "lppch.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	FrameBuffer::FrameBuffer()
	{
		glGenFramebuffers(1, &m_RendererID);
	
		glGenTextures(1, &m_Texture);
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, )
	}

	FrameBuffer::~FrameBuffer()
	{}

	void FrameBuffer::Bind()
	{}

	void FrameBuffer::Unbind()
	{}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create()
	{
		return std::shared_ptr<FrameBuffer>();
	}
}