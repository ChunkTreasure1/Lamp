#include "lppch.h"
#include "Brush.h"

#include "BrushManager.h"
#include "Lamp/Objects/ObjectLayer.h"
#include <btBulletDynamicsCommon.h>
#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	static btVector3 ToBtVector(const glm::vec3& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	Brush::Brush(Ref<Model> model)
		: m_Model(model)
	{
		m_pRigidBody = PhysicsEngine::Get()->CreateRigidBody(this);

		btConvexHullShape* pShape = new btConvexHullShape();
		glm::vec3 points[8];

		points[0] = m_Model->GetBoundingBox().Min;
		points[1] = m_Model->GetBoundingBox().Max;
		points[2] = { points[0].x, points[0].y, points[1].z };
		points[3] = { points[0].x, points[1].y, points[0].z };
		points[4] = { points[1].x, points[0].y, points[0].z };
		points[5] = { points[0].x, points[1].y, points[1].z };
		points[6] = { points[1].x, points[0].y, points[1].z };
		points[7] = { points[1].x, points[1].y, points[0].z };

		pShape->addPoint(ToBtVector(points[0]));
		pShape->addPoint(ToBtVector(points[1]));
		pShape->addPoint(ToBtVector(points[2]));
		pShape->addPoint(ToBtVector(points[3]));
		pShape->addPoint(ToBtVector(points[4]));
		pShape->addPoint(ToBtVector(points[5]));
		pShape->addPoint(ToBtVector(points[6]));
		pShape->addPoint(ToBtVector(points[7]));

		m_pRigidBody->SetCollisionShape(pShape, this);

		m_pRigidBody->GetRigidbody()->setUserPointer(this);
		m_pRigidBody->GetCollisionShape()->setUserPointer(this);
		m_pRigidBody->SetStatic(true);

		m_Name = "Brush";
	}

	void Brush::UpdatedMatrix()
	{
		m_Model->SetModelMatrix(m_ModelMatrix);
	}

	void Brush::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush::OnRender));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Brush::OnUpdate));
	}

	void Brush::Destroy()
	{
		g_pEnv->pBrushManager->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	void Brush::ScaleChanged()
	{
		m_Model->GetBoundingBox().Max = m_Scale * m_Model->GetBoundingBox().StartMax;
		m_Model->GetBoundingBox().Min = m_Scale * m_Model->GetBoundingBox().StartMin;
	}

	Brush* Brush::Create(const std::string& path)
	{
		return g_pEnv->pBrushManager->Create(path);
	}

	Brush* Brush::Duplicate(Brush* main)
	{
		Brush* pBrush = g_pEnv->pBrushManager->Create(main);
		pBrush->SetPosition(main->GetPosition());
		pBrush->SetRotation(main->GetRotation());
		pBrush->SetScale(main->GetScale());

		std::string name = main->GetName();
		if (auto pos = name.find_last_of("1234567890"); pos != std::string::npos)
		{
			int currIndex = stoi(name.substr(pos, 1));
			currIndex++;
			name.replace(pos, 1, std::to_string(currIndex));
		}

		pBrush->SetName(name);
	
		return pBrush;
	}

	bool Brush::OnRender(AppRenderEvent& e)
	{
		if (e.GetPassInfo().type == PassType::Forward)
		{
			return true;
		}

		bool forward = e.GetPassInfo().type == PassType::Selection ? true : false;

		m_Model->Render(m_Id, forward);

		if (g_pEnv->ShouldRenderBB && e.GetPassInfo().type == PassType::Forward)
		{
			m_Model->RenderBoundingBox();
		}

		return true;
	}

	bool Brush::OnUpdate(AppUpdateEvent& e)
	{
		btTransform& tr = m_pRigidBody->GetTransform();
		SetPhysicsPosition({ tr.getOrigin().getX(), tr.getOrigin().getY(), tr.getOrigin().getZ() });

		return false;
	}
}