#pragma once

#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"

#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	class RigidbodyComponent final : public EntityComponent
	{
	public:
		RigidbodyComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<RigidbodyComponent>(); }
		static std::string GetFactoryName() { return "RigidbodyComponent"; }

		enum class Type : uint32_t
		{
			Static, Dynamic
		};

		enum class CollisionDetectionType : uint32_t
		{
			Discrete, Continuous, ContinuousSpeculative
		};

		struct RigidbodySpecification
		{
			Type m_BodyType = Type::Dynamic;
			CollisionDetectionType m_CollisionDetection = CollisionDetectionType::Discrete;

			float m_Mass = 1.f;
			float m_LinearDrag = 1.f;
			float m_AngularDrag = 1.f;

			bool m_DisableGravity = false;
			bool IsKinematic = false;
			uint32_t m_Layer = 0;
		};

		inline RigidbodySpecification& GetSpecification() { return m_Specification; }

	private:
		RigidbodySpecification m_Specification;
	};
}