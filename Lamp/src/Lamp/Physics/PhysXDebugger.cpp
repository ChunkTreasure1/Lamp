#include "lppch.h"
#include "PhysXDebugger.h"

#include "PhysXInternal.h"

namespace Lamp
{
	struct PhysXData
	{
		physx::PxPvd* pDebugger;
		physx::PxPvdTransport* pTransport;
	};

	static PhysXData* s_pData = nullptr;

#ifdef LP_DEBUG
	void PhysXDebugger::Initialize()
	{
		s_pData = new PhysXData();

		s_pData->pDebugger = PxCreatePvd(PhysXInternal::GetFoundation());
		LP_CORE_ASSERT(s_pData->pDebugger, "PxCreatePvd failed!");
	}

	void PhysXDebugger::Shutdown()
	{
		s_pData->pDebugger->release();
		delete s_pData;
		s_pData = nullptr;
	}

	void PhysXDebugger::StartDebugging(const std::string& filepath, bool networkDebug)
	{
		StopDebugging();
		if (!networkDebug)
		{
			s_pData->pTransport = physx::PxDefaultPvdFileTransportCreate((filepath + ".pxd2").c_str());
			s_pData->pDebugger->connect(*s_pData->pTransport, physx::PxPvdInstrumentationFlag::eALL);
		}
		else
		{
			s_pData->pTransport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 1000);
			s_pData->pDebugger->connect(*s_pData->pTransport, physx::PxPvdInstrumentationFlag::eALL);
		}
	}

	bool PhysXDebugger::IsDebugging()
	{
		return s_pData->pDebugger->isConnected();
	}

	void PhysXDebugger::StopDebugging()
	{
		if (!s_pData->pDebugger->isConnected())
		{
			return;
		}

		s_pData->pDebugger->disconnect();
		s_pData->pTransport->release();
	}

	physx::PxPvd* PhysXDebugger::GetDebugger()
	{
		return s_pData->pDebugger;
	}
#else
	void PhysXDebugger::Initialize() {}
	void PhysXDebugger::Shutdown() {}
	void PhysXDebugger::StartDebugging(const std::string& filepath, bool networkDebug) {}
	bool PhysXDebugger::IsDebugging() { return false; }
	void PhysXDebugger::StopDebugging() {}
	physx::PxPvd* PhysXDebugger::GetDebugger() { return nullptr; }
#endif
}