#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Physics/Rigidbody.h"

#include "Lamp/Meshes/Model.h"
#include "Lamp/Objects/Object.h"

namespace Lamp
{
	class Brush : public Object
	{
	public:
		Brush(Ref<Model> model);

		virtual void UpdatedMatrix() override;
		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;
		virtual uint64_t GetEventMask() override { return EventType::AppRender | EventType::AppUpdate; }

		//Getting
		inline Ref<Model>& GetModel() { return m_Model; }
		inline Rigidbody* GetRigidbody() { return m_pRigidBody; }

	protected:
		virtual void ScaleChanged() override;

	public:
		static Brush* Create(const std::string& path);

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);

	private:
		Ref<Model> m_Model;
	};
}