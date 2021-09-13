#include "lppch.h"
#include "CookingFactory.h"
#include "PhysXInternal.h"

namespace Lamp
{
    struct CookingData
    {
        physx::PxCooking* CookingSDK;
        physx::PxCookingParams CookingParams;

        CookingData(const physx::PxTolerancesScale& scale)
            : CookingSDK(nullptr), CookingParams(scale)
        {}
    };

    static CookingData* s_CookingData = nullptr;

    void CookingFactory::Initialize()
    {
        s_CookingData = new CookingData(PhysXInternal::GetPhysXSDK().getTolerancesScale());
        s_CookingData->CookingParams.meshWeldTolerance = 0.1f;
        s_CookingData->CookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
        s_CookingData->CookingParams.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

        s_CookingData->CookingSDK = PxCreateCooking(PX_PHYSICS_VERSION, PhysXInternal::GetFoundation(), s_CookingData->CookingParams);
    }

    void CookingFactory::Shutdown()
    {
        s_CookingData->CookingSDK->release();
        s_CookingData->CookingSDK = nullptr;

        delete s_CookingData;
    }
    CookingResult CookingFactory::CookMesh(Ref<MeshColliderComponent> component, bool invalidateOld, std::vector<MeshColliderData>& outData)
    {
        return CookingResult();
    }
    CookingResult CookingFactory::CookConvexMesh(const Ref<Model>& mesh, std::vector<MeshColliderData>& outData)
    {
        return CookingResult();
    }
    CookingResult CookingFactory::CookTriangleMesh(const Ref<Model>& mesh, std::vector<MeshColliderData>& outData)
    {
        return CookingResult();
    }
}