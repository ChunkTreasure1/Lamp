#include "lppch.h"
#include "PhysXInternal.h"

#include "Lamp/Objects/Entity/BaseComponents/Physics/RigidbodyComponent.h"

namespace Lamp
{
	struct PhysXData
	{
		physx::PxFoundation* PhysXFoundation;
		physx::PxDefaultCpuDispatcher* PhysXCPUDispatcher;
		physx::PxPhysics* PhysXSDK;

		physx::PxDefaultAllocator Allocator;
		PhysicsErrorCallback ErrorCallback;
		PhysicsAssertHandler AssertHandler;
	};

	static PhysXData* s_PhysXData;

	void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorMessage = NULL;

		switch (code)
		{
		case physx::PxErrorCode::eNO_ERROR:				errorMessage = "No Error"; break;
		case physx::PxErrorCode::eDEBUG_INFO:			errorMessage = "Info"; break;
		case physx::PxErrorCode::eDEBUG_WARNING:		errorMessage = "Warning"; break;
		case physx::PxErrorCode::eINVALID_PARAMETER:	errorMessage = "Invalid Parameter"; break;
		case physx::PxErrorCode::eINVALID_OPERATION:	errorMessage = "Invalid Operation"; break;
		case physx::PxErrorCode::eOUT_OF_MEMORY:		errorMessage = "Out Of Memory"; break;
		case physx::PxErrorCode::eINTERNAL_ERROR:		errorMessage = "Internal Error"; break;
		case physx::PxErrorCode::eABORT:				errorMessage = "Abort"; break;
		case physx::PxErrorCode::ePERF_WARNING:			errorMessage = "Performance Warning"; break;
		case physx::PxErrorCode::eMASK_ALL:				errorMessage = "Unknown Error"; break;
		}

		switch (code)
		{
		case physx::PxErrorCode::eNO_ERROR:
		case physx::PxErrorCode::eDEBUG_INFO:
			LP_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
			break;
		case physx::PxErrorCode::eDEBUG_WARNING:
		case physx::PxErrorCode::ePERF_WARNING:
			LP_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
			break;
		case physx::PxErrorCode::eINVALID_PARAMETER:
		case physx::PxErrorCode::eINVALID_OPERATION:
		case physx::PxErrorCode::eOUT_OF_MEMORY:
		case physx::PxErrorCode::eINTERNAL_ERROR:
			LP_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
			break;
		case physx::PxErrorCode::eABORT:
		case physx::PxErrorCode::eMASK_ALL:
			LP_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
			LP_ASSERT(false, "");
			break;
		}
	}

	void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
	{
		LP_ERROR("[PhysX Error]: {0}:{1} - {2}", file, line, exp);
	}

	void PhysXInternal::Initialize()
	{
		LP_CORE_ASSERT(!s_PhysXData, "PhysX should only be initialized once!");
		
		s_PhysXData = new PhysXData();
		s_PhysXData->PhysXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_PhysXData->Allocator, s_PhysXData->ErrorCallback);
		LP_CORE_ASSERT(s_PhysXData->PhysXFoundation, "PxCreateFoundation failed");

		physx::PxTolerancesScale scale = physx::PxTolerancesScale();
		scale.length = 1.f;
		scale.speed = 100.f;

		//TODO: Physics debugger

#ifdef LP_DEBUG
		static bool s_TrackMemoryAllocations = true;
#else
		static bool s_TrackMemoryAllocations = false;
#endif

		s_PhysXData->PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *s_PhysXData->PhysXFoundation, scale, s_TrackMemoryAllocations);
		LP_CORE_ASSERT(s_PhysXData->PhysXSDK, "PxCreatePhysX failed");

		bool extentionsLoaded = PxInitExtensions(*s_PhysXData->PhysXSDK, nullptr);
		LP_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX extentions");

		s_PhysXData->PhysXCPUDispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		PxSetAssertHandler(s_PhysXData->AssertHandler);
		
		//TODO: Cooking factory
	}

	void PhysXInternal::Shutdown()
	{
		s_PhysXData->PhysXCPUDispatcher->release();
		s_PhysXData->PhysXCPUDispatcher = nullptr;

		PxCloseExtensions();

		s_PhysXData->PhysXSDK->release();
		s_PhysXData->PhysXSDK = nullptr;

		s_PhysXData->PhysXFoundation->release();
		s_PhysXData->PhysXFoundation = nullptr;

		delete s_PhysXData;
		s_PhysXData = nullptr;
	}

	physx::PxFoundation& PhysXInternal::GetFoundation()
	{
		return *s_PhysXData->PhysXFoundation;
	}

	physx::PxPhysics& PhysXInternal::GetPhysXSDK()
	{
		return *s_PhysXData->PhysXSDK;
	}

	physx::PxCpuDispatcher* PhysXInternal::GetCPUDispatcher()
	{
		return s_PhysXData->PhysXCPUDispatcher;
	}


	physx::PxDefaultAllocator& PhysXInternal::GetAllocator()
	{
		return s_PhysXData->Allocator;
	}

	physx::PxFilterFlags PhysXInternal::FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 == (uint32_t)RigidbodyComponent::CollisionDetectionType::Continuous || filterData1.word2 == (uint32_t)RigidbodyComponent::CollisionDetectionType::Continuous)
		{
			pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		}

		if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eSUPPRESS;
	}

	physx::PxBroadPhaseType::Enum PhysXInternal::LampToPhysXBroadphaseType(BroadphaseType type)
	{
		switch (type)
		{
		case Lamp::BroadphaseType::SweepAndPrune: return physx::PxBroadPhaseType::eSAP;
		case Lamp::BroadphaseType::MutliBoxPrune: return physx::PxBroadPhaseType::eMBP;
		case Lamp::BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
		default:
			break;
		}

		return physx::PxBroadPhaseType::eABP;
	}

	physx::PxFrictionType::Enum PhysXInternal::LampToPhysXFrictionType(FrictionType type)
	{
		switch (type)
		{
		case Lamp::FrictionType::Patch: return physx::PxFrictionType::ePATCH;
		case Lamp::FrictionType::OneDirectional: return physx::PxFrictionType::eONE_DIRECTIONAL;
		case Lamp::FrictionType::TwoDirectional: return physx::PxFrictionType::eTWO_DIRECTIONAL;
		default:
			break;
		}

		return physx::PxFrictionType::ePATCH;
	}
}