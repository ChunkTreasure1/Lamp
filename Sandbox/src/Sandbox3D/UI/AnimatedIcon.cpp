#include "AnimatedIcon.h"
#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox3D
{
	AnimatedIcon::AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime)
		: m_AnimTime(animTime), m_FrameCount(frameCount), m_PerFrameTime(animTime / (float)frameCount)
	{
		std::string filename = firstFrame.stem().string();
		size_t numPos = filename.find_first_of("0123456789");
		if (numPos != std::string::npos)
		{
			filename = filename.substr(0, numPos);
		}

		std::filesystem::path dirPath = firstFrame.parent_path();

		for (uint32_t frame = 1; frame <= frameCount; frame++)
		{
			std::filesystem::path path = dirPath / std::filesystem::path(filename + std::to_string(frame) + firstFrame.extension().string());
			m_Textures.push_back(Lamp::ResourceCache::GetAsset<Lamp::Texture2D>(path));
		}

		LP_CORE_ASSERT(!m_Textures.empty(), "No frames found!");
		LP_CORE_ASSERT(m_Textures.size() == frameCount, "Not all frames loaded!");
		m_CurrentTexture = m_Textures[0];
	}

	bool AnimatedIcon::Animate(Lamp::AppUpdateEvent& e)
	{
		if (!m_Playing)
		{
			return false;
		}

		m_CurrTime += e.GetTimestep();
		if (m_CurrTime > m_PerFrameTime)
		{
			m_CurrFrame++;
			m_CurrTime = 0.f;
			if (m_CurrFrame >= m_FrameCount)
			{
				m_CurrFrame = 0;
			}

			m_CurrentTexture = m_Textures[m_CurrFrame];
		}

		return false;
	}

	void AnimatedIcon::OnEvent(Lamp::Event& e)
	{
		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(AnimatedIcon::Animate));
	}
}