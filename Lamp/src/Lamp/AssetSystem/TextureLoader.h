#pragma once

namespace Lamp
{
	enum class TextureType
	{
		Texture2D,
		HDR
	};

	struct TextureLoadData
	{
		std::string path;
		uint32_t width;
		uint32_t height;
		uint32_t channels;

		void* pData = nullptr;
		TextureType type;
	};

	struct TextureData
	{
		uint32_t rendererId;
		uint32_t width;
		uint32_t height;

		uint32_t internalFormat;
		uint32_t dataFormat;
	};

	class TextureLoader
	{
	public:
		static void LoadTexture(TextureLoadData& data, const std::string& path);
		static void LoadHDRTexture(TextureLoadData& data, const std::string& path);
		static TextureData GenerateTexture(const TextureLoadData& texture);
		static TextureData GenerateHDR(const TextureLoadData& texture);
	};
}