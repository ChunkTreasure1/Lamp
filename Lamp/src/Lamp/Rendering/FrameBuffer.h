#pragma once

namespace Lamp
{
	class FrameBuffer
	{
	public:
		FrameBuffer();
		~FrameBuffer();

		void Bind();
		void Unbind();

	public:
		static std::shared_ptr<FrameBuffer> Create();

	private:
		uint32_t m_RendererID;
		uint32_t m_Texture;
	};
}