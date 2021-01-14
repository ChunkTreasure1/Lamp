#include "lppch.h"
#include "PhysicsEngine.h"
#include "Rigidbody.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	PhysicsEngine* PhysicsEngine::s_pPhysicsEngine = nullptr;

	PhysicsEngine::PhysicsEngine()
	{
		s_pPhysicsEngine = this;
	}

	PhysicsEngine::~PhysicsEngine()
	{
	}

	void PhysicsEngine::Initialize()
	{
		m_pCollisionConfig = new btDefaultCollisionConfiguration();
		m_pCollisionDispatcher = new btCollisionDispatcher(m_pCollisionConfig);
		m_pOverlappingPairCache = new btDbvtBroadphase();
		m_pSolver = new btSequentialImpulseConstraintSolver();
		m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pCollisionDispatcher, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfig);

		m_pDynamicsWorld->setGravity(btVector3(0.f, -9.81f, 0.f));
		//m_pDynamicsWorld->setInternalTickCallback(TickCallback);
	}

	void PhysicsEngine::Shutdown()
	{
		for (int i = m_pDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			m_pDynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}

		delete m_pDynamicsWorld;
		delete m_pSolver;
		delete m_pOverlappingPairCache;
		delete m_pCollisionDispatcher;
	}

	void PhysicsEngine::Simulate()
	{
		m_pDynamicsWorld->stepSimulation(1.f / 60.f, 10);
	}

	void PhysicsEngine::Update()
	{

	}

	void PhysicsEngine::UpdateAABBs()
	{
		m_pDynamicsWorld->updateAabbs();
	}

	Rigidbody* PhysicsEngine::CreateRigidBody(Object* obj)
	{
		Rigidbody* rb = new Rigidbody();
		glm::quat q(obj->GetRotation());

		btCollisionShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

		btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(q.x, q.y, q.z, q.w), ToBtVector3(obj->GetPosition())));
		btRigidBody::btRigidBodyConstructionInfo cI(1.f, motionState, shape, btVector3(1.f, 1.f, 1.f));

		rb->SetRigidBody(new btRigidBody(cI));
		rb->SetCollisionShape(shape, obj);
		
		m_pDynamicsWorld->addRigidBody(rb->GetRigidbody());

		rb->GetRigidbody()->setFriction(1.f);
		rb->SetDamping(0.f, 0.f);

		m_pBodies.push_back(rb);
		return rb;
	}

	void PhysicsEngine::RemoveRigidBody(Object* obj)
	{
		if (auto it = std::find(m_pBodies.begin(), m_pBodies.end(), obj->GetRigidbody()); it != m_pBodies.end())
		{
			m_pBodies.erase(it);
		}
	}

	bool PhysicsEngine::RayTest(const glm::vec3& origin, const glm::vec3& dir, float range)
	{
		glm::vec3 outEnd = origin + dir * range;
		btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z));

		m_pDynamicsWorld->rayTest(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z), RayCallback);

		return RayCallback.hasHit();
	}

	bool PhysicsEngine::Raycast(const glm::vec3& origin, const glm::vec3& dir, float range, RayHit* hit)
	{
		glm::vec3 outEnd = origin + dir * range;
		btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z));
		m_pDynamicsWorld->rayTest(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z), RayCallback);

		bool hasHit = RayCallback.hasHit();
		hit->hit = hasHit;

		if (hasHit)
		{
			hit->point.x = RayCallback.m_hitPointWorld.getX();
			hit->point.y = RayCallback.m_hitPointWorld.getY();
			hit->point.z = RayCallback.m_hitPointWorld.getZ();

			hit->normal.x = RayCallback.m_hitNormalWorld.getX();
			hit->normal.y = RayCallback.m_hitNormalWorld.getY();
			hit->normal.z = RayCallback.m_hitNormalWorld.getZ();

			hit->distance = glm::distance(origin, hit->point);
			hit->pObject = (Object*)RayCallback.m_collisionObject->getUserPointer();
		}

		return hasHit;
	}

	Object* PhysicsEngine::RaycastRef(const glm::vec3& origin, const glm::vec3& dir, float range)
	{
		glm::vec3 outEnd = origin + dir * range;
		btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z));
		m_pDynamicsWorld->rayTest(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z), RayCallback);

		bool hasHit = RayCallback.hasHit();

		if (hasHit)
		{
			return (Object*)RayCallback.m_collisionObject->getUserPointer();
		}

		return nullptr;
	}

	bool PhysicsEngine::RaycastHitPoint(const glm::vec3& origin, const glm::vec3& dir, float range, glm::vec3& point)
	{
		glm::vec3 outEnd = origin + dir * range;
		btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z));
		m_pDynamicsWorld->rayTest(btVector3(origin.x, origin.y, origin.z), btVector3(outEnd.x, outEnd.y, outEnd.z), RayCallback);

		bool hasHit = RayCallback.hasHit();

		if (hasHit)
		{
			point = glm::vec3(RayCallback.m_hitPointWorld.getX(), RayCallback.m_hitPointWorld.getY(), RayCallback.m_hitPointWorld.getZ());
			return true;
		}

		return false;
	}

	bool PhysicsEngine::CheckSphere(const glm::vec3& center, float radius)
	{
		return false;
	}

	void PhysicsEngine::ScreenPosToWorldRay(const glm::vec2& mousePos, const glm::vec2& screenSize, const glm::mat4& view, const glm::mat4& proj, glm::vec3& outOrigin, glm::vec3& outDir)
	{
		glm::vec4 start((mousePos.x / screenSize.x - 0.5f) * 2.0f, -(mousePos.y / screenSize.y - 0.5f) * 2.f, -1.f, 1.f);
		glm::vec4 end((mousePos.x / screenSize.x - 0.5f) * 2.f, -(mousePos.y / screenSize.y - 0.5f) * 2.f, 0.f, 1.f);
	
		glm::mat4 m = glm::inverse(proj * view);
		
		glm::vec4 startWorld = m * start;
		startWorld /= startWorld.w;

		glm::vec4 endWorld = m * end;
		endWorld /= endWorld.w;

		glm::vec3 dir(endWorld - startWorld);
		dir = glm::normalize(dir);

		outOrigin = glm::vec3(startWorld);
		outDir = dir;
	}

	void PhysicsEngine::TickCallback(btDynamicsWorld* pWorld, btScalar timestep)
	{

	}

	btVector3 PhysicsEngine::ToBtVector3(const glm::vec3& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	glm::vec3 PhysicsEngine::ToVec3(const btVector3& v)
	{
		return glm::vec3(v.getX(), v.getY(), v.getZ());
	}
}