#include "lppch.h"
#include "Brush.h"

#include "BrushManager.h"
#include "Lamp/Objects/ObjectLayer.h"
#include <btBulletDynamicsCommon.h>
#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	Brush::Brush(Ref<Model> model)
		: m_Model(model)
	{
		m_PickingCollider = PickingCollider(m_Model->GetBoundingBox().Min, m_Model->GetBoundingBox().Max, m_Position);

		m_pRigidBody = PhysicsEngine::Get()->CreateRigidBody(this);
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
		BrushManager::Get()->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	void Brush::ScaleChanged()
	{
		m_Model->GetBoundingBox().Max = m_Scale * m_Model->GetBoundingBox().StartMax;
		m_Model->GetBoundingBox().Min = m_Scale * m_Model->GetBoundingBox().StartMin;
		
		m_PickingCollider = PickingCollider(m_Model->GetBoundingBox().Min, m_Model->GetBoundingBox().Max, m_Position);
	}

	Brush* Brush::Create(const std::string& path)
	{
		return BrushManager::Get()->Create(path);
	}

	bool Brush::OnRender(AppRenderEvent& e)
	{
		m_Model->Render();

		if (g_pEnv->ShouldRenderBB && !e.GetPassInfo().IsShadowPass)
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