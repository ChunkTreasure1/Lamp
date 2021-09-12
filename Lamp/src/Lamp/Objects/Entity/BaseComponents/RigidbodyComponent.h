#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	class RigidbodyComponent final : public EntityComponent
	{
	public:
		RigidbodyComponent()
			: EntityComponent("RigidbodyComponent")
		{
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override { return EventType::None; }
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<RigidbodyComponent>(); }
		static std::string GetFactoryName() { return "RigidbodyComponent"; }

	public:
		enum class Type
		{
			Static, Dynamic
		};

		enum class CollisionDetectionType : uint32_t
		{
			Discrete, Continuous, ContinuousSpeculative
		};

	private:
		Type m_BodyType = Type::Dynamic;
		CollisionDetectionType m_CollisionDetection = CollisionDetectionType::Discrete;

		float m_Mass = 1.f;
		float m_LinearDrag = 1.f;
		float AngularDrag = 1.f;

		bool m_DisableGravity = false;
		bool IsKinematic = false;
		uint32_t m_Layer = 0;
	};
}