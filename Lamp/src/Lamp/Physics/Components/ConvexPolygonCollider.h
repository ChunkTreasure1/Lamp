#pragma once

#include "Lamp/Entity/Base/BaseComponent.h"

#include <vector>

namespace Lamp
{
	class ConvexPolygonColliderComponent : public EntityComponent
	{
	public:
		ConvexPolygonColliderComponent()
			: EntityComponent("ConvexPolygonColliderComponent"), m_VerticeCount(4), m_SideLength(1), m_Tag(""), m_Mass(0.f)
		{}

		////////Base////////
		virtual void Initialize() override;
		virtual void OnEvent(Lamp::Event& e) override {}
		////////////////////
		
		//Getting
		inline std::vector<glm::vec2>& GetVertices() { return m_VerticesWorld; }
		inline std::vector<Ref<ConvexPolygonColliderComponent>>& GetCurrentColliders() { return m_CurrentColliders; }
		inline const bool GetIsColliding() { return m_IsColliding; }

		inline const bool GetShouldInteract() { return m_ShouldInteract; }
		inline const std::string& GetTag() { return m_Tag; }
		inline const float GetMass() { return m_Mass; }

		//Setting
		inline void SetVerticeCount(uint32_t count) 
		{ 
			m_VerticeCount = count; 
			m_VerticesModel.clear();
			m_VerticesWorld.clear();

			float angle = 3.14159f * 2.f / m_VerticeCount;
			for (size_t i = 0; i < m_VerticeCount; i++)
			{
				m_VerticesModel.push_back({ m_SideLength * cosf(angle * i), m_SideLength * sinf(angle * i) });
				m_VerticesWorld.push_back({ m_SideLength * cosf(angle * i), m_SideLength * sinf(angle * i) });
			}
		}
		inline void SetSideLength(float val) { m_SideLength = val; }
		inline void SetIsColliding(bool state) { m_IsColliding = state; }

		inline void SetShouldInteract(bool state) { m_ShouldInteract = state; }
		inline void SetVerticePositions(std::initializer_list<glm::vec2> pos) { m_VerticesModel = pos; m_VerticesWorld = pos; }
		inline void SetTag(const std::string& tag) { m_Tag = tag; }

		inline void SetMass(float mass) { m_Mass = mass; }

	public:
		static Ref<EntityComponent> Create() { return std::make_shared<ConvexPolygonColliderComponent>(); }
		static std::string GetFactoryName() { return "ConvexPolygonColliderComponent"; }

	private:
		static bool s_Registered;

	private:
		uint32_t m_VerticeCount;
		std::string m_Tag;

		float m_SideLength;
		float m_Mass;
		bool m_IsColliding = false;
		bool m_ShouldInteract = true;

		std::vector<glm::vec2> m_VerticesModel;
		std::vector<glm::vec2> m_VerticesWorld;
		std::vector<Ref<ConvexPolygonColliderComponent>> m_CurrentColliders;
	};
}