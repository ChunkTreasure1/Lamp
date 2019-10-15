#pragma once

namespace Lamp
{
	class TextureCache
	{
	public:
		static const std::tuple<uint32_t, uint32_t, uint32_t> GetTexture(const std::string& path);

	private:
		static std::map<std::string, std::tuple<uint32_t, uint32_t, uint32_t>> m_TextureCache;
	};
}