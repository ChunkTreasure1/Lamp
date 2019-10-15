#pragma once

#include <string>

namespace Lamp
{
	class Texture2D
	{
	public:
		Texture2D(const std::string& path);

		void Bind();

		//Getting
		inline const uint32_t GetWidth() const { return m_Width; }
		inline const uint32_t GetHeight() const { return m_Height; }
		inline const uint32_t GetID() const { return m_RendererID; }

	public:
		static Texture2D* Create(const std::string& path) { return new Texture2D(path); }

	private:
		uint32_t m_RendererID;
		uint32_t m_Width;
		uint32_t m_Height;
	};
}