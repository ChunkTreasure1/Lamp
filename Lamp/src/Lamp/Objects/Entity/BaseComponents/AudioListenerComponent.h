#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Audio/AudioEngine.h"

namespace Lamp
{
	class AudioListenerComponent final : public EntityComponent
	{
	public:
		AudioListenerComponent()
			: EntityComponent("AudioListenerComponent"), m_Forward(0.f), m_Up(0.f), m_Listener(0)
		{
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint32_t GetSize() { return sizeof(*this); }
		virtual uint64_t GetEventMask() { return EventType::EntityPositionChanged; }
		//////////////

	private:
		void CalculateForwardAndUp();
		bool OnPositionChanged(EntityPositionChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<AudioListenerComponent>(); }
		static std::string GetFactoryName() { return "AudioListenerComponent"; }

	private:
		glm::vec3 m_Forward;
		glm::vec3 m_Up;
	
		Listener m_Listener;
	};
}