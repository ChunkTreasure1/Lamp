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
			m_perspectiveCamera = CreateRef<PerspectiveCamera>(60.f, 0.1f, 100.f);
		}
		~CameraComponent() {}

		//////Base//////
		void Initialize() override;
		void OnEvent(Lamp::Event& someE) override;
		////////////////

		inline void SetIsMain(bool state) { m_isMain = state; }
		inline bool GetIsMain() { return m_isMain; }
		inline Ref<CameraBase> GetCamera() { return std::dynamic_pointer_cast<CameraBase>(m_perspectiveCamera); }

	private:
		bool OnUpdate(AppUpdateEvent& e);
		bool OnViewportSizeChanged(EditorViewportSizeChangedEvent& e);
		bool OnPropertyUpdated(ObjectPropertyChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<CameraComponent>(); }
		static std::string GetFactoryName() { return "CameraComponent"; }

	private:
		bool m_isPerspective = true;
		bool m_isMain = false;

		float m_nearPlane = 0.01f;
		float m_farPlane = 1000.f;

		float m_currentAspectRatio = 1.7f;

		Ref<PerspectiveCamera> m_perspectiveCamera;
		Ref<OrthographicCamera> m_OrthographicCamera;
	};
}