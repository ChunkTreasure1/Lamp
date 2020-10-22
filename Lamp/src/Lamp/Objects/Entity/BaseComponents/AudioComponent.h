#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

namespace Lamp
{
	class AudioComponent final : public EntityComponent
	{
	public:
		AudioComponent()
			: EntityComponent("AudioComponent"), m_EventName(""), m_PlayOnStart(false)
		{
			SetComponentProperties
			({
				{ PropertyType::String, "Event name", RegisterData(&m_EventName) },
				{ PropertyType::Bool, "Play on start", RegisterData(&m_PlayOnStart) }
			});
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override { return EventType::None; }
		//////////////

		//Setting
		inline void SetEvent(const std::string& name) { m_EventName = name; }

		//Getting
		inline const std::string& GetEvent() { return m_EventName; }

		void Play();
		void Stop();

	public:
		static Ref<EntityComponent> Create() { return CreateRef<AudioComponent>(); }
		static std::string GetFactoryName() { return "AudioComponent"; }

	private:
		std::string m_EventName;
		bool m_PlayOnStart;
	};
}