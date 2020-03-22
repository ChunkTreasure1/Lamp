#include "lppch.h"
#include "SpriteAnimationComponent.h"

#include <Lamp/Rendering/Renderer2D.h>
#include <Lamp/Entity/Base/Entity.h>
#include <Lamp/Entity/Base/ComponentRegistry.h>

namespace Game
{
	bool SpriteAnimationComponent::s_Registered = LP_REGISTER_COMPONENT(SpriteAnimationComponent);

	void SpriteAnimationComponent::Initialize()
	{
	}

	void SpriteAnimationComponent::Update(Lamp::Timestep myTS)
	{
		myCurrentTime += myTS.GetMilliseconds();

		if (myAnimTimeCurrent < myCurrentTime)
		{
			if (myCurrentTexture == myCurrentSelectedAnimSet->second.end() - 1)
			{
				myCurrentTexture = myCurrentSelectedAnimSet->second.begin();
			}
			else 
			{
				myCurrentTexture++;
			}

			myAnimTimeCurrent = myAnimTime + myCurrentTime;
		}
	}

	void SpriteAnimationComponent::OnEvent(Lamp::Event& e)
	{
	}

	void SpriteAnimationComponent::Draw()
	{
		if (myTextures.size() < 1)
		{
			return;
		}

		Lamp::Renderer2D::DrawQuad(m_pEntity->GetPosition(), m_pEntity->GetScale(), m_pEntity->GetRotation().x, *myCurrentTexture);
	}

	void SpriteAnimationComponent::AddAnimation(const std::string& someType, Ref<Lamp::Texture2D>& someTexture)
	{
		if (auto tempIt = myTextures.find(someType); tempIt == myTextures.end())
		{
			std::vector<Ref<Lamp::Texture2D>> tempVec;
			tempVec.push_back(someTexture);

			myTextures.insert(std::make_pair(someType, std::move(tempVec)));

			return;
		}
		else
		{
			tempIt->second.push_back(someTexture);
		}
	}

	void SpriteAnimationComponent::AddAnimation(std::pair<std::string, std::initializer_list<Ref<Lamp::Texture2D>>> someTextures)
	{
		std::vector<Ref<Lamp::Texture2D>> tempVec = someTextures.second;

		if (auto tempIt = myTextures.find(someTextures.first); tempIt != myTextures.end())
		{
			for (size_t i = 0; i < tempVec.size(); i++)
			{
				tempIt->second.push_back(std::move(tempVec[i]));
			}
		
			return;
		}

		myTextures.insert(someTextures);
	}

	bool SpriteAnimationComponent::SetCurrentAnimSet(const std::string& someType)
	{
		if (myCurrentSelectedAnimSet != myTextures.end())
		{
			if (myCurrentSelectedAnimSet->first == someType)
			{
				return false;
			}
		}

		if (auto tempIt = myTextures.find(someType); tempIt != myTextures.end())
		{
			myCurrentSelectedAnimSet = tempIt;
			myCurrentTexture = tempIt->second.begin();
			return true;
		}

		return false;
	}
}