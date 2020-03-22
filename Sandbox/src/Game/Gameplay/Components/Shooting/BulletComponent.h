#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Rendering/Texture2D/Texture2D.h>

namespace Game
{
	class BulletComponent final : public Lamp::IEntityComponent
	{
	public:
		BulletComponent()
			: IEntityComponent("BulletComponent")
		{}
		~BulletComponent() {}

		////////Base////////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep myTS) override;
		virtual void OnEvent(Lamp::Event& e) override;
		virtual void Draw() override;
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////////

		inline void SetDirection(const glm::vec3& tempDir) { myDirection = tempDir; }

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<BulletComponent>(); }
		static std::string GetFactoryName() { return "BulletComponent"; }

	private:
		static bool s_Registered;

	private:
		glm::vec3 myDirection;
		Ref<Lamp::Texture2D> myTexture;
	};
}