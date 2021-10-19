#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"
#include "Lamp/Rendering/Cameras/OrthographicCamera.h"
#include "Lamp/Event/EditorEvent.h"

#include <Lamp.h>

namespace Lamp
{
	class CameraComponent : public EntityComponent
	{
	public:

		CameraComponent()
			: EntityComponent("CameraComponent")
		{
			m_PerspectiveCamera = CreateRef<PerspectiveCamera>(60.f, 0.1f, 100.f);
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
		bool OnViewportSizeChanged(EditorViewportSizeChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<CameraComponent>(); }
		static std::string GetFactoryName() { return "CameraComponent"; }

	private:
		bool m_IsPerspective = true;
		bool m_IsMain = false;

		Ref<PerspectiveCamera> m_PerspectiveCamera;
		Ref<OrthographicCamera> m_OrthographicCamera;
	};
}