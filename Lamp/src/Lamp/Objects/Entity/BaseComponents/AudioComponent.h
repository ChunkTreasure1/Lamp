#pragma once

#include "Lamp/Objects/Entity/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Entity.h"

namespace Lamp
{
	class AudioComponent final : public EntityComponent
	{
	public:
		AudioComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

		inline void SetEvent(const std::string& name) { m_EventName = name; }
		inline const std::string& GetEvent() { return m_EventName; }

		void Play();
		void Stop();

		static Ref<EntityComponent> Create() { return CreateRef<AudioComponent>(); }
		static std::string GetFactoryName() { return "AudioComponent"; }

	private:
		std::string m_EventName;
		bool m_PlayOnStart;
	};
}
