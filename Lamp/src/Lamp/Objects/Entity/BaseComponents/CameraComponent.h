#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"
#include "Lamp/Rendering/Cameras/OrthographicCamera.h"

#include <Lamp.h>

namespace Lamp
{
	class CameraComponent : public EntityComponent
	{
	public:

		CameraComponent()
			: EntityComponent("CameraComponent")
		{
			m_PerspectiveCamera = std::make_shared<PerspectiveCamera>(60.f, 0.1, 100.f);
		}
		~CameraComponent() {}

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Lamp::Event& someE) override;
		////////////////

		inline void SetIsMain(bool state) { m_IsMain = state; }
		inline bool GetIsMain() { return m_IsMain; }
		inline Ref<CameraBase> GetCamera() { return std::dynamic_pointer_cast<CameraBase>(m_PerspectiveCamera); }

	private:
		bool OnUpdate(AppUpdateEvent& e);

	public:
		static Ref<EntityComponent> Create() { return std::make_shared<CameraComponent>(); }
		static std::string GetFactoryName() { return "CameraComponent"; }

	private:
		static bool s_Registered;

	private:
		bool m_IsPerspective = true;
		bool m_IsMain = false;

		Ref<PerspectiveCamera> m_PerspectiveCamera;
		Ref<OrthographicCamera> m_OrthographicCamera;
	};
}