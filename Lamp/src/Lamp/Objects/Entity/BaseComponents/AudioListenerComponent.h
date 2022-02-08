#pragma once

#include "Lamp/Objects/Entity/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Entity.h"
#include "Lamp/Audio/AudioEngine.h"

namespace Lamp
{
	class AudioListenerComponent final : public EntityComponent
	{
	public:
		AudioListenerComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		//////////////

		static Ref<EntityComponent> Create() { return CreateRef<AudioListenerComponent>(); }
		static std::string GetFactoryName() { return "AudioListenerComponent"; }

	private:
		void CalculateForwardAndUp();
		bool OnPositionChanged(ObjectPositionChangedEvent& e);

		glm::vec3 m_forward;
		glm::vec3 m_up;
	
		Listener m_listener;
	};
}