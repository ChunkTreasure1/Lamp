#pragma once

#include <string>

#include <glad/glad.h>
#include "Lamp/Core/Core.h"

namespace Lamp
{
	enum class TextureType
	{
		Diffuse,
		Specular
	};

	class Texture2D
	{
	public:
		Texture2D(const std::string& path);
		Texture2D(uint32_t width, uint32_t height);

		void Bind();
		void Unbind();

		void SetData(void* data, uint32_t size);
		void SetType(TextureType type) { m_Type = type; }

		//Getting
		inline const uint32_t GetWidth() const { return m_Width; }
		inline const uint32_t GetHeight() const { return m_Height; }
		inline const uint32_t GetID() const { return m_RendererID; }

		inline const TextureType GetType() const { return m_Type; }

	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height) { return std::make_shared<Texture2D>(width, height); }
		static Ref<Texture2D> Create(const std::string& path) { return std::make_shared<Texture2D>(path); }

	private:
		uint32_t m_RendererID;
		uint32_t m_Width;
		uint32_t m_Height;

		TextureType m_Type;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}