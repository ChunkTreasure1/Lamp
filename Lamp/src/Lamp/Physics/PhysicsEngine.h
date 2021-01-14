#pragma once

#include <btBulletDynamicsCommon.h>
#include "Lamp/Objects/Object.h"

namespace Lamp
{
	class Rigidbody;

	struct RayHit
	{
		bool hit;
		float distance;
		glm::vec3 point;
		glm::vec3 normal;
		Object* pObject;
	};

	class PhysicsEngine
	{
	public:
		PhysicsEngine();
		~PhysicsEngine();

		void Initialize();
		void Shutdown();
		void Simulate();
		void Update();
		void UpdateAABBs();

		//Rigidbodies
		Rigidbody* CreateRigidBody(Object* obj);
		void AddRigidBody(Object* obj);
		void RemoveRigidBody(Object* obj);

		//Rays
		bool RayTest(const glm::vec3& origin, const glm::vec3& dir, float range);
		bool Raycast(const glm::vec3& origin, const glm::vec3& dir, float range, RayHit* hit);
		Object* RaycastRef(const glm::vec3& origin, const glm::vec3& dir, float range);
		bool RaycastHitPoint(const glm::vec3& origin, const glm::vec3& dir, float range, glm::vec3& point);

		bool CheckSphere(const glm::vec3& center, float radius);
		void ScreenPosToWorldRay(const glm::vec2& mousePos, const glm::vec2& screenSize, const glm::mat4& view, const glm::mat4& proj, glm::vec3& outOrigin, glm::vec3& outDir);

	private:
		void TickCallback(btDynamicsWorld* pWorld, btScalar timestep);

	public:
		static PhysicsEngine* Get() { return s_pPhysicsEngine; }
		static btVector3 ToBtVector3(const glm::vec3& v);
		static glm::vec3 ToVec3(const btVector3& v);
		
	private:
		static PhysicsEngine* s_pPhysicsEngine;

	private:
		btDefaultCollisionConfiguration* m_pCollisionConfig = nullptr;
		btCollisionDispatcher* m_pCollisionDispatcher = nullptr;
		btBroadphaseInterface* m_pOverlappingPairCache = nullptr;
		btSequentialImpulseConstraintSolver* m_pSolver = nullptr;
		btDiscreteDynamicsWorld* m_pDynamicsWorld = nullptr;
	
		std::vector<Rigidbody*> m_pBodies;
	};
}