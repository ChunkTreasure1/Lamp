#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Rendering/Texture2D/Texture2D.h>
#include <unordered_map>
#include <vector>

namespace Game
{
	class SpriteAnimationComponent final : public Lamp::IEntityComponent
	{
	public:
		SpriteAnimationComponent()
			: IEntityComponent("SpriteAnimationComponent"), myCurrentSelectedAnimSet(), myTextures(), myAnimTime(0.f)
		{
			myCurrentSelectedAnimSet = myTextures.end();
		}
		~SpriteAnimationComponent() {}

		////////Base////////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep myTS) override;
		virtual void OnEvent(Lamp::Event& e) override;
		virtual void Draw() override;
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////////

		void AddAnimation(const std::string& someType, Ref<Lamp::Texture2D>& someTextures);
		void AddAnimation(std::pair<std::string, std::initializer_list<Ref<Lamp::Texture2D>>> someTextures);

		//Setting
		bool SetCurrentAnimSet(const std::string& someType);
		inline void SetAnimationTime(float someTime) { myAnimTime = someTime; }

	public:
		static Ref<IEntityComponent> Create() { return std::shared_ptr<SpriteAnimationComponent>(); }
		static std::string GetFactoryName() { return "SpriteAnimationComponent"; }

	private:
		static bool s_Registered;

	private:
		std::vector<Ref<Lamp::Texture2D>>::iterator myCurrentTexture;
		std::unordered_map<std::string, std::vector<Ref<Lamp::Texture2D>>>::iterator myCurrentSelectedAnimSet;
		std::unordered_map<std::string, std::vector<Ref<Lamp::Texture2D>>> myTextures;
		
		float myAnimTime;
		float myCurrentTime;
		float myAnimTimeCurrent;
	};
}