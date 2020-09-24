#pragma once

namespace Lamp
{
	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Update(const uint32_t width, const uint32_t height) = 0;

		virtual inline const uint32_t GetColorAttachment() const = 0;
		virtual inline const uint32_t GetDepthAttachment() const = 0;

	public:
		static Ref<FrameBuffer> Create(const uint32_t width, const uint32_t height);
	};
}