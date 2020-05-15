#pragma once

namespace Lamp
{
	class FrameBuffer
	{
	public:
		FrameBuffer(const uint32_t width, const uint32_t height);
		FrameBuffer(const uint32_t width, const uint32_t height, bool state);

		~FrameBuffer();

		void Bind();
		void Unbind();
		void Update(const uint32_t width, const uint32_t height);

		inline uint32_t GetTexture() const { return m_Texture; }

	public:
		static std::shared_ptr<FrameBuffer> Create(const uint32_t width, const uint32_t height);
		static std::shared_ptr<FrameBuffer> CreateShadowBuffer(const uint32_t, const uint32_t);

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_ColorID;

		uint32_t m_Texture;
		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;
	};
}