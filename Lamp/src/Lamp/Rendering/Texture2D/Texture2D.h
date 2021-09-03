#pragma once

#include <string>

#include <glad/glad.h>
#include "Lamp/Core/Core.h"

#include "Lamp/AssetSystem/TextureLoader.h"

namespace Lamp
{
	class Texture2D
	{
	public:
		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetData(TextureData& data) = 0;
		void SetType(TextureType type) { m_Type = type; }

		//Getting
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		inline const TextureType GetType() const { return m_Type; }
		inline std::string& GetPath() { return m_Path; }

	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);

	protected:
		TextureType m_Type;
		std::string m_Path;
	};
}