#pragma once

namespace Lamp
{
	class FrameBuffer
	{
	public:
		FrameBuffer(const uint32_t width, const uint32_t height);

		~FrameBuffer();

		void Bind();
		void Unbind();
		void Update(const uint32_t width, const uint32_t height);

		inline const uint32_t GetColorAttachment() const { return m_ColorID; }
		inline const uint32_t GetDepthAttachment() const { return m_DepthID; }

	public:
		static std::shared_ptr<FrameBuffer> Create(const uint32_t width, const uint32_t height);

	private:
		void Invalidate();
		
	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_ColorID;

		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;
	};
}