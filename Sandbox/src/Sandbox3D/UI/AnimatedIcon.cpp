#include "AnimatedIcon.h"
#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox3D
{
	AnimatedIcon::AnimatedIcon(uint32_t frameCount, const std::filesystem::path& firstFrame)
	{
		std::string filename = firstFrame.stem().string();
		size_t numPos = filename.find_first_of("0123456789");
		if (numPos != std::string::npos)
		{
			filename = filename.substr(0, numPos - 1);
		}

		std::filesystem::path dirPath = firstFrame.parent_path();

		for (uint32_t frame = 0; frame < frameCount; frame++)
		{
			m_Textures.push_back(Lamp::ResourceCache::GetAsset<Lamp::Texture2D>(dirPath / std::filesystem::path(filename + std::to_string(frame))));
		}
	}

	void AnimatedIcon::Animate(Lamp::Timestep ts)
	{

	}
}